#!/usr/bin/env node

/**
 * @file create-app.js
 * @brief CLI tool for creating new T-Embed CC1101 apps
 */

const fs = require('fs');
const path = require('path');
const { program } = require('commander');
const inquirer = require('inquirer');
const chalk = require('chalk');
const { TEmbedSDK, PERMISSIONS } = require('../index.js');

const sdk = new TEmbedSDK();

program
  .name('create-app')
  .description('Create a new T-Embed CC1101 JavaScript app')
  .version('1.0.0')
  .option('-n, --name <name>', 'app name')
  .option('-d, --dir <directory>', 'target directory', './apps')
  .option('-t, --template <template>', 'app template (basic, rf, ui, gpio)', 'basic')
  .option('-i, --interactive', 'interactive mode')
  .action(async (options) => {
    try {
      let config = {};

      if (options.interactive || !options.name) {
        config = await promptForConfig();
      } else {
        config = {
          name: options.name,
          template: options.template,
          targetDir: options.dir
        };
      }

      await createApp(config);
    } catch (error) {
      console.error(chalk.red('Error:'), error.message);
      process.exit(1);
    }
  });

async function promptForConfig() {
  console.log(chalk.blue('🚀 T-Embed CC1101 App Creator'));
  console.log('');

  const answers = await inquirer.prompt([
    {
      type: 'input',
      name: 'name',
      message: 'App name:',
      validate: (input) => {
        if (!input || input.trim().length === 0) {
          return 'App name is required';
        }
        if (!/^[a-zA-Z][a-zA-Z0-9_-]*$/.test(input.trim())) {
          return 'App name must start with a letter and contain only letters, numbers, hyphens, and underscores';
        }
        return true;
      },
      filter: (input) => input.trim()
    },
    {
      type: 'input',
      name: 'description',
      message: 'Description:',
      default: (answers) => `${answers.name} application for T-Embed CC1101`
    },
    {
      type: 'input',
      name: 'author',
      message: 'Author:',
      default: 'Developer'
    },
    {
      type: 'list',
      name: 'template',
      message: 'Choose a template:',
      choices: [
        { name: 'Basic - Simple app with UI', value: 'basic' },
        { name: 'RF - Radio communication app', value: 'rf' },
        { name: 'UI - Advanced user interface', value: 'ui' },
        { name: 'GPIO - Hardware control app', value: 'gpio' }
      ]
    },
    {
      type: 'list',
      name: 'category',
      message: 'App category:',
      choices: [
        { name: 'Utility - General purpose tools', value: 'utility' },
        { name: 'RF - Radio frequency applications', value: 'rf' },
        { name: 'Hardware - GPIO and hardware control', value: 'hardware' },
        { name: 'Entertainment - Games and fun apps', value: 'entertainment' },
        { name: 'Development - Developer tools', value: 'development' }
      ]
    },
    {
      type: 'checkbox',
      name: 'permissions',
      message: 'Select required permissions:',
      choices: [
        { name: 'RF Receive - Read radio signals', value: PERMISSIONS.RF_RECEIVE },
        { name: 'RF Transmit - Send radio signals', value: PERMISSIONS.RF_TRANSMIT },
        { name: 'GPIO Read - Read GPIO pins', value: PERMISSIONS.GPIO_READ },
        { name: 'GPIO Write - Control GPIO pins', value: PERMISSIONS.GPIO_WRITE },
        { name: 'Storage Read - Read files', value: PERMISSIONS.STORAGE_READ },
        { name: 'Storage Write - Write files', value: PERMISSIONS.STORAGE_WRITE },
        { name: 'UI Create - Create user interfaces', value: PERMISSIONS.UI_CREATE },
        { name: 'Network - Network access', value: PERMISSIONS.NETWORK },
        { name: 'System - System functions', value: PERMISSIONS.SYSTEM }
      ]
    },
    {
      type: 'input',
      name: 'targetDir',
      message: 'Target directory:',
      default: './apps'
    }
  ]);

  return answers;
}

async function createApp(config) {
  console.log('');
  console.log(chalk.yellow('Creating app...'));

  // Validate target directory
  const fullTargetDir = path.resolve(config.targetDir);
  const appDir = path.join(fullTargetDir, config.name);

  if (fs.existsSync(appDir)) {
    throw new Error(`Directory already exists: ${appDir}`);
  }

  // Create app using SDK
  sdk.createApp(config.name, config.targetDir, {
    description: config.description,
    author: config.author,
    template: config.template,
    category: config.category,
    permissions: config.permissions || []
  });

  // Create default icon
  createDefaultIcon(appDir);

  // Create .gitignore
  createGitignore(appDir);

  console.log('');
  console.log(chalk.green('✅ App created successfully!'));
  console.log('');
  console.log(chalk.blue('Next steps:'));
  console.log(`  cd ${path.relative(process.cwd(), appDir)}`);
  console.log('  # Edit app.js to implement your app logic');
  console.log('  # Add assets to the assets/ directory');
  console.log('  # Test your app using the T-Embed device or simulator');
  console.log('');
  console.log(chalk.blue('Useful commands:'));
  console.log('  npm run validate-app     # Validate app structure');
  console.log('  npm run build-app        # Build app for deployment');
  console.log('  npm run install-app      # Install app to device');
}

function createDefaultIcon(appDir) {
  // Create a simple SVG icon
  const iconContent = `<?xml version="1.0" encoding="UTF-8"?>
<svg width="64" height="64" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg">
  <rect width="64" height="64" fill="#3498db" rx="8"/>
  <text x="32" y="40" font-family="Arial, sans-serif" font-size="36" fill="white" text-anchor="middle">📱</text>
</svg>`;

  fs.writeFileSync(path.join(appDir, 'assets', 'app.svg'), iconContent);

  // Create a simple PNG placeholder description
  const iconInfo = `# App Icon

Place your app icon files here:
- app.png (64x64 pixels, recommended)
- app.svg (vector format, optional)

The icon will be displayed in the app launcher and app manager.

## Guidelines

- Use clear, simple designs
- Ensure good contrast
- Test at different sizes
- Follow material design principles
`;

  fs.writeFileSync(path.join(appDir, 'assets', 'README.md'), iconInfo);
}

function createGitignore(appDir) {
  const gitignoreContent = `# Build output
build/
dist/

# Temporary files
*.tmp
.temp/

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS files
.DS_Store
Thumbs.db

# Logs
*.log
logs/

# App specific
app.min.js
`;

  fs.writeFileSync(path.join(appDir, '.gitignore'), gitignoreContent);
}

if (require.main === module) {
  program.parse();
}