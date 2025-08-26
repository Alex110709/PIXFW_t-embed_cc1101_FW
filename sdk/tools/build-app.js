#!/usr/bin/env node

/**
 * @file build-app.js
 * @brief CLI tool for building T-Embed CC1101 apps for deployment
 */

const fs = require('fs');
const path = require('path');
const { program } = require('commander');
const chalk = require('chalk');
const fsExtra = require('fs-extra');
const minify = require('node-json-minify');

program
  .name('build-app')
  .description('Build a T-Embed CC1101 JavaScript app for deployment')
  .version('1.0.0')
  .argument('<app-path>', 'path to app directory')
  .option('-o, --output <path>', 'output directory', './build')
  .option('-m, --minify', 'minify JavaScript code')
  .option('-c, --compress', 'create compressed archive')
  .option('-v, --verbose', 'verbose output')
  .action(async (appPath, options) => {
    try {
      await buildApp(appPath, options);
    } catch (error) {
      console.error(chalk.red('Error:'), error.message);
      process.exit(1);
    }
  });

async function buildApp(appPath, options) {
  const fullAppPath = path.resolve(appPath);
  const appName = path.basename(fullAppPath);
  
  if (!fs.existsSync(fullAppPath)) {
    throw new Error(`App directory not found: ${fullAppPath}`);
  }

  console.log(chalk.blue('🔨 Building app:'), appName);
  console.log('');

  // Validate app first
  const validation = await validateAppForBuild(fullAppPath);
  if (!validation.valid) {
    throw new Error(`App validation failed: ${validation.errors.join(', ')}`);
  }

  // Create output directory
  const outputDir = path.resolve(options.output);
  const appOutputDir = path.join(outputDir, appName);
  
  if (fs.existsSync(appOutputDir)) {
    if (options.verbose) {
      console.log(chalk.yellow('Cleaning existing build directory...'));
    }
    fsExtra.removeSync(appOutputDir);
  }

  fsExtra.ensureDirSync(appOutputDir);

  // Build process
  const buildInfo = {
    appName,
    sourcePath: fullAppPath,
    outputPath: appOutputDir,
    timestamp: new Date().toISOString(),
    options
  };

  console.log(chalk.yellow('📦 Processing files...'));

  // Copy and process manifest
  await processManifest(fullAppPath, appOutputDir, buildInfo, options);

  // Copy and process main JS file
  await processMainFile(fullAppPath, appOutputDir, buildInfo, options);

  // Copy assets
  await processAssets(fullAppPath, appOutputDir, buildInfo, options);

  // Copy additional files
  await processAdditionalFiles(fullAppPath, appOutputDir, buildInfo, options);

  // Generate build info
  await generateBuildInfo(appOutputDir, buildInfo, options);

  // Create installation package
  if (options.compress) {
    await createInstallationPackage(appOutputDir, buildInfo, options);
  }

  console.log('');
  console.log(chalk.green('✅ Build completed successfully!'));
  console.log(chalk.blue('Output:'), path.relative(process.cwd(), appOutputDir));

  if (options.verbose) {
    displayBuildSummary(buildInfo);
  }
}

async function validateAppForBuild(appPath) {
  const errors = [];

  // Check required files
  const manifestPath = path.join(appPath, 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    errors.push('Missing manifest.json');
  } else {
    try {
      const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
      
      if (!manifest.main) {
        errors.push('Missing main file in manifest');
      } else {
        const mainPath = path.join(appPath, manifest.main);
        if (!fs.existsSync(mainPath)) {
          errors.push(`Main file not found: ${manifest.main}`);
        }
      }
    } catch (e) {
      errors.push(`Invalid manifest.json: ${e.message}`);
    }
  }

  return {
    valid: errors.length === 0,
    errors
  };
}

async function processManifest(sourcePath, outputPath, buildInfo, options) {
  const manifestPath = path.join(sourcePath, 'manifest.json');
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

  // Add build information to manifest
  manifest._build = {
    timestamp: buildInfo.timestamp,
    sdk_version: require('../package.json').version,
    minified: options.minify || false
  };

  // Write processed manifest
  const manifestContent = options.minify 
    ? minify(JSON.stringify(manifest))
    : JSON.stringify(manifest, null, 2);

  fs.writeFileSync(path.join(outputPath, 'manifest.json'), manifestContent);

  if (options.verbose) {
    console.log(chalk.green('  ✅ Processed manifest.json'));
  }
}

async function processMainFile(sourcePath, outputPath, buildInfo, options) {
  const manifestPath = path.join(sourcePath, 'manifest.json');
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  
  const mainFile = manifest.main || 'app.js';
  const mainPath = path.join(sourcePath, mainFile);
  
  let content = fs.readFileSync(mainPath, 'utf8');

  // Add build header
  const buildHeader = `// Built with T-Embed CC1101 SDK v${require('../package.json').version}
// Build time: ${buildInfo.timestamp}
// App: ${buildInfo.appName}

`;

  if (options.minify) {
    // Simple minification (remove comments and extra whitespace)
    content = minifyJavaScript(content);
    if (options.verbose) {
      console.log(chalk.green('  ✅ Minified JavaScript code'));
    }
  }

  content = buildHeader + content;

  fs.writeFileSync(path.join(outputPath, mainFile), content);

  if (options.verbose) {
    console.log(chalk.green(`  ✅ Processed ${mainFile}`));
  }
}

async function processAssets(sourcePath, outputPath, buildInfo, options) {
  const assetsPath = path.join(sourcePath, 'assets');
  
  if (fs.existsSync(assetsPath)) {
    const outputAssetsPath = path.join(outputPath, 'assets');
    fsExtra.copySync(assetsPath, outputAssetsPath);
    
    if (options.verbose) {
      const files = fs.readdirSync(assetsPath);
      console.log(chalk.green(`  ✅ Copied ${files.length} asset(s)`));
    }
  }
}

async function processAdditionalFiles(sourcePath, outputPath, buildInfo, options) {
  const additionalFiles = ['README.md', 'LICENSE', 'CHANGELOG.md'];
  
  for (const file of additionalFiles) {
    const filePath = path.join(sourcePath, file);
    if (fs.existsSync(filePath)) {
      fsExtra.copySync(filePath, path.join(outputPath, file));
      if (options.verbose) {
        console.log(chalk.green(`  ✅ Copied ${file}`));
      }
    }
  }
}

async function generateBuildInfo(outputPath, buildInfo, options) {
  const buildInfoContent = {
    app_name: buildInfo.appName,
    build_timestamp: buildInfo.timestamp,
    sdk_version: require('../package.json').version,
    build_options: {
      minified: options.minify || false,
      compressed: options.compress || false
    },
    source_path: buildInfo.sourcePath,
    node_version: process.version,
    platform: process.platform
  };

  fs.writeFileSync(
    path.join(outputPath, '.build-info.json'),
    JSON.stringify(buildInfoContent, null, 2)
  );

  if (options.verbose) {
    console.log(chalk.green('  ✅ Generated build info'));
  }
}

async function createInstallationPackage(appPath, buildInfo, options) {
  console.log(chalk.yellow('📦 Creating installation package...'));

  // Create install script
  const installScript = generateInstallScript(buildInfo);
  fs.writeFileSync(path.join(appPath, 'install.sh'), installScript);
  fs.chmodSync(path.join(appPath, 'install.sh'), '755');

  // Create Windows install script
  const installBat = generateInstallBatch(buildInfo);
  fs.writeFileSync(path.join(appPath, 'install.bat'), installBat);

  // Create package info
  const packageInfo = generatePackageInfo(buildInfo);
  fs.writeFileSync(path.join(appPath, 'PACKAGE.md'), packageInfo);

  if (options.verbose) {
    console.log(chalk.green('  ✅ Created installation package'));
  }
}

function minifyJavaScript(code) {
  // Simple JavaScript minification
  return code
    // Remove single-line comments
    .replace(/\/\/.*$/gm, '')
    // Remove multi-line comments
    .replace(/\/\*[\s\S]*?\*\//g, '')
    // Remove extra whitespace
    .replace(/\s+/g, ' ')
    // Remove whitespace around operators
    .replace(/\s*([{}();,])\s*/g, '$1')
    .trim();
}

function generateInstallScript(buildInfo) {
  return `#!/bin/bash

# T-Embed CC1101 App Installer
# App: ${buildInfo.appName}
# Generated: ${buildInfo.timestamp}

set -e

APP_NAME="${buildInfo.appName}"
DEVICE_PATH="/dev/ttyUSB0"
TARGET_DIR="/apps"

echo "Installing T-Embed CC1101 App: $APP_NAME"

# Check if device is connected
if [ ! -e "$DEVICE_PATH" ]; then
    echo "Error: Device not found at $DEVICE_PATH"
    echo "Please connect your T-Embed device and try again."
    exit 1
fi

# Upload files to device
echo "Uploading app files..."

# Create app directory on device
echo "mkdir -p $TARGET_DIR/$APP_NAME" > "$DEVICE_PATH"

# Upload manifest
echo "Uploading manifest.json..."
cat manifest.json > "$DEVICE_PATH"

# Upload main app file
MAIN_FILE=$(grep '"main"' manifest.json | cut -d'"' -f4)
echo "Uploading $MAIN_FILE..."
cat "$MAIN_FILE" > "$DEVICE_PATH"

# Upload assets if they exist
if [ -d "assets" ]; then
    echo "Uploading assets..."
    find assets -type f | while read file; do
        echo "Uploading $file..."
        cat "$file" > "$DEVICE_PATH"
    done
fi

echo "Installation completed successfully!"
echo "Restart your T-Embed device to see the new app."
`;
}

function generateInstallBatch(buildInfo) {
  return `@echo off
REM T-Embed CC1101 App Installer
REM App: ${buildInfo.appName}
REM Generated: ${buildInfo.timestamp}

set APP_NAME=${buildInfo.appName}
set DEVICE_PATH=COM3
set TARGET_DIR=/apps

echo Installing T-Embed CC1101 App: %APP_NAME%

REM Check if device is connected
if not exist "%DEVICE_PATH%" (
    echo Error: Device not found at %DEVICE_PATH%
    echo Please connect your T-Embed device and update the DEVICE_PATH variable.
    pause
    exit /b 1
)

echo Uploading app files...

REM Upload files (this is a simplified example)
echo Upload process would be implemented here
echo Please refer to the documentation for manual installation instructions.

pause
`;
}

function generatePackageInfo(buildInfo) {
  return `# ${buildInfo.appName} Installation Package

This package contains a T-Embed CC1101 JavaScript application ready for installation.

## Package Contents

- \`manifest.json\` - App metadata and configuration
- \`app.js\` - Main application code (or as specified in manifest)
- \`assets/\` - App resources (icons, images, etc.)
- \`.build-info.json\` - Build information
- \`install.sh\` - Linux/macOS installation script
- \`install.bat\` - Windows installation script

## Installation Methods

### Automatic Installation (Linux/macOS)

\`\`\`bash
chmod +x install.sh
./install.sh
\`\`\`

### Manual Installation

1. Connect your T-Embed CC1101 device
2. Copy the app directory to \`/apps/${buildInfo.appName}/\` on the device
3. Restart the device

### Using Web IDE

1. Connect to the device's web interface
2. Upload the app files through the file manager
3. Install the app using the app manager

## Build Information

- **App Name:** ${buildInfo.appName}
- **Build Time:** ${buildInfo.timestamp}
- **SDK Version:** ${require('../package.json').version}
- **Built with minification:** ${buildInfo.options.minify ? 'Yes' : 'No'}

## Support

For help and documentation, visit:
https://github.com/your-repo/t-embed-cc1101-fw
`;
}

function displayBuildSummary(buildInfo) {
  console.log('');
  console.log(chalk.blue('📊 Build Summary:'));
  console.log(`  App Name: ${buildInfo.appName}`);
  console.log(`  Source: ${path.relative(process.cwd(), buildInfo.sourcePath)}`);
  console.log(`  Output: ${path.relative(process.cwd(), buildInfo.outputPath)}`);
  console.log(`  Minified: ${buildInfo.options.minify ? 'Yes' : 'No'}`);
  console.log(`  Compressed: ${buildInfo.options.compress ? 'Yes' : 'No'}`);
  console.log(`  Build Time: ${buildInfo.timestamp}`);
}

if (require.main === module) {
  program.parse();
}