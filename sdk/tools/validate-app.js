#!/usr/bin/env node

/**
 * @file validate-app.js
 * @brief CLI tool for validating T-Embed CC1101 apps
 */

const fs = require('fs');
const path = require('path');
const { program } = require('commander');
const chalk = require('chalk');
const { TEmbedSDK } = require('../index.js');

const sdk = new TEmbedSDK();

program
  .name('validate-app')
  .description('Validate a T-Embed CC1101 JavaScript app')
  .version('1.0.0')
  .argument('<app-path>', 'path to app directory')
  .option('-v, --verbose', 'verbose output')
  .option('-f, --fix', 'attempt to fix common issues')
  .action(async (appPath, options) => {
    try {
      await validateApp(appPath, options);
    } catch (error) {
      console.error(chalk.red('Error:'), error.message);
      process.exit(1);
    }
  });

async function validateApp(appPath, options) {
  const fullPath = path.resolve(appPath);
  
  if (!fs.existsSync(fullPath)) {
    throw new Error(`App directory not found: ${fullPath}`);
  }

  if (!fs.statSync(fullPath).isDirectory()) {
    throw new Error(`Path is not a directory: ${fullPath}`);
  }

  console.log(chalk.blue('🔍 Validating app:'), path.basename(fullPath));
  console.log('');

  // Perform validation
  const result = sdk.validateApp(fullPath);
  
  // Additional checks
  const additionalChecks = performAdditionalChecks(fullPath, options);
  result.errors.push(...additionalChecks.errors);
  result.warnings.push(...additionalChecks.warnings);

  // Display results
  displayResults(result, options);

  // Attempt fixes if requested
  if (options.fix && result.errors.length > 0) {
    await attemptFixes(fullPath, result, options);
  }

  // Exit with appropriate code
  if (result.errors.length > 0) {
    process.exit(1);
  }
}

function performAdditionalChecks(appPath, options) {
  const errors = [];
  const warnings = [];

  // Check file structure
  const expectedFiles = ['manifest.json'];
  const expectedDirs = ['assets'];
  
  for (const file of expectedFiles) {
    if (!fs.existsSync(path.join(appPath, file))) {
      errors.push(`Missing required file: ${file}`);
    }
  }

  for (const dir of expectedDirs) {
    if (!fs.existsSync(path.join(appPath, dir))) {
      warnings.push(`Missing recommended directory: ${dir}`);
    }
  }

  // Check manifest.json syntax and content
  const manifestPath = path.join(appPath, 'manifest.json');
  if (fs.existsSync(manifestPath)) {
    try {
      const manifestContent = fs.readFileSync(manifestPath, 'utf8');
      const manifest = JSON.parse(manifestContent);

      // Check manifest version compatibility
      if (manifest.manifest_version !== '1.0') {
        warnings.push(`Unsupported manifest version: ${manifest.manifest_version}`);
      }

      // Check for main file
      if (manifest.main) {
        const mainPath = path.join(appPath, manifest.main);
        if (!fs.existsSync(mainPath)) {
          errors.push(`Main file not found: ${manifest.main}`);
        } else {
          // Check main file syntax
          const syntaxCheck = checkJavaScriptSyntax(mainPath);
          if (!syntaxCheck.valid) {
            errors.push(`Syntax error in ${manifest.main}: ${syntaxCheck.error}`);
          }
        }
      }

      // Check icon file
      if (manifest.icon) {
        const iconPath = path.join(appPath, 'assets', manifest.icon);
        if (!fs.existsSync(iconPath)) {
          warnings.push(`Icon file not found: assets/${manifest.icon}`);
        }
      }

      // Check version format
      if (manifest.version && !isValidVersion(manifest.version)) {
        warnings.push(`Invalid version format: ${manifest.version} (expected semantic versioning)`);
      }

      // Check name format
      if (manifest.name && !isValidAppName(manifest.name)) {
        errors.push(`Invalid app name: ${manifest.name} (must be alphanumeric with hyphens/underscores)`);
      }

    } catch (e) {
      errors.push(`Invalid JSON in manifest.json: ${e.message}`);
    }
  }

  // Check for common files
  const commonFiles = ['README.md', '.gitignore'];
  for (const file of commonFiles) {
    if (!fs.existsSync(path.join(appPath, file))) {
      warnings.push(`Missing recommended file: ${file}`);
    }
  }

  // Check app.js for common patterns
  const mainFile = path.join(appPath, 'app.js');
  if (fs.existsSync(mainFile)) {
    const content = fs.readFileSync(mainFile, 'utf8');
    
    // Check for initialization function
    if (!content.includes('function init') && !content.includes('const init') && !content.includes('let init')) {
      warnings.push('No init() function found in main file');
    }

    // Check for proper API usage
    const apiChecks = checkAPIUsage(content);
    warnings.push(...apiChecks);
  }

  return { errors, warnings };
}

function checkJavaScriptSyntax(filePath) {
  try {
    const content = fs.readFileSync(filePath, 'utf8');
    
    // Basic syntax check using eval (in a try-catch)
    // Note: This is a simplified check. In production, use a proper JS parser
    try {
      new Function(content);
      return { valid: true };
    } catch (syntaxError) {
      return { valid: false, error: syntaxError.message };
    }
  } catch (error) {
    return { valid: false, error: `Cannot read file: ${error.message}` };
  }
}

function isValidVersion(version) {
  // Check semantic versioning pattern (major.minor.patch)
  const semverPattern = /^\d+\.\d+\.\d+(-[a-zA-Z0-9.-]+)?(\+[a-zA-Z0-9.-]+)?$/;
  return semverPattern.test(version);
}

function isValidAppName(name) {
  // Check app name pattern
  const namePattern = /^[a-zA-Z][a-zA-Z0-9_-]*$/;
  return namePattern.test(name);
}

function checkAPIUsage(content) {
  const warnings = [];

  // Check for deprecated patterns
  if (content.includes('document.') || content.includes('window.location')) {
    warnings.push('Browser-specific APIs detected (document, window.location) - these are not available on T-Embed');
  }

  // Check for Node.js patterns
  if (content.includes('require(') || content.includes('process.')) {
    warnings.push('Node.js specific APIs detected - these are not available on T-Embed');
  }

  // Check for proper error handling
  const apiCalls = ['RF.', 'GPIO.', 'Storage.', 'UI.'];
  let hasApiCalls = false;
  let hasErrorHandling = false;

  for (const api of apiCalls) {
    if (content.includes(api)) {
      hasApiCalls = true;
      break;
    }
  }

  if (content.includes('try {') || content.includes('catch(') || content.includes('.catch(')) {
    hasErrorHandling = true;
  }

  if (hasApiCalls && !hasErrorHandling) {
    warnings.push('API calls detected but no error handling found - consider adding try/catch blocks');
  }

  return warnings;
}

function displayResults(result, options) {
  const totalIssues = result.errors.length + result.warnings.length;

  if (totalIssues === 0) {
    console.log(chalk.green('✅ App validation passed!'));
    console.log('No issues found.');
    return;
  }

  // Display errors
  if (result.errors.length > 0) {
    console.log(chalk.red(`❌ ${result.errors.length} error(s) found:`));
    result.errors.forEach((error, index) => {
      console.log(chalk.red(`  ${index + 1}. ${error}`));
    });
    console.log('');
  }

  // Display warnings
  if (result.warnings.length > 0) {
    console.log(chalk.yellow(`⚠️  ${result.warnings.length} warning(s) found:`));
    result.warnings.forEach((warning, index) => {
      console.log(chalk.yellow(`  ${index + 1}. ${warning}`));
    });
    console.log('');
  }

  // Summary
  if (result.errors.length > 0) {
    console.log(chalk.red('Validation failed. Please fix the errors above.'));
  } else {
    console.log(chalk.yellow('Validation passed with warnings.'));
  }

  if (options.verbose) {
    console.log('');
    console.log(chalk.blue('Validation summary:'));
    console.log(`  Total issues: ${totalIssues}`);
    console.log(`  Errors: ${result.errors.length}`);
    console.log(`  Warnings: ${result.warnings.length}`);
  }
}

async function attemptFixes(appPath, result, options) {
  console.log('');
  console.log(chalk.yellow('🔧 Attempting to fix issues...'));

  let fixedCount = 0;

  // Fix missing directories
  if (result.errors.some(e => e.includes('Missing recommended directory: assets'))) {
    const assetsDir = path.join(appPath, 'assets');
    if (!fs.existsSync(assetsDir)) {
      fs.mkdirSync(assetsDir, { recursive: true });
      console.log(chalk.green('  ✅ Created assets/ directory'));
      fixedCount++;
    }
  }

  // Create missing README.md
  if (result.warnings.some(w => w.includes('Missing recommended file: README.md'))) {
    const readmePath = path.join(appPath, 'README.md');
    if (!fs.existsSync(readmePath)) {
      const appName = path.basename(appPath);
      const readmeContent = `# ${appName}

A T-Embed CC1101 JavaScript application.

## Description

[Add your app description here]

## Usage

[Add usage instructions here]

## Development

This app was created using the T-Embed CC1101 SDK.

## License

MIT
`;
      fs.writeFileSync(readmePath, readmeContent);
      console.log(chalk.green('  ✅ Created README.md'));
      fixedCount++;
    }
  }

  // Create missing .gitignore
  if (result.warnings.some(w => w.includes('Missing recommended file: .gitignore'))) {
    const gitignorePath = path.join(appPath, '.gitignore');
    if (!fs.existsSync(gitignorePath)) {
      const gitignoreContent = `# Build output
build/
dist/

# Temporary files
*.tmp
.temp/

# IDE files
.vscode/
.idea/

# OS files
.DS_Store
Thumbs.db
`;
      fs.writeFileSync(gitignorePath, gitignoreContent);
      console.log(chalk.green('  ✅ Created .gitignore'));
      fixedCount++;
    }
  }

  if (fixedCount > 0) {
    console.log('');
    console.log(chalk.green(`✅ Fixed ${fixedCount} issue(s)`));
    console.log(chalk.blue('Run validation again to check remaining issues.'));
  } else {
    console.log(chalk.yellow('No auto-fixable issues found.'));
  }
}

if (require.main === module) {
  program.parse();
}