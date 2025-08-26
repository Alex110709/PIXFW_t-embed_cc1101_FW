#!/usr/bin/env node

/**
 * @file cli.js
 * @brief Main CLI interface for T-Embed CC1101 SDK
 */

const { program } = require('commander');
const chalk = require('chalk');
const path = require('path');
const fs = require('fs');

const packageInfo = require('./package.json');

program
  .name('t-embed-sdk')
  .description('T-Embed CC1101 JavaScript SDK CLI')
  .version(packageInfo.version);

// Create app command
program
  .command('create')
  .alias('new')
  .description('Create a new T-Embed app')
  .argument('[name]', 'app name')
  .option('-d, --dir <directory>', 'target directory', './apps')
  .option('-t, --template <template>', 'app template (basic, rf, ui, gpio)', 'basic')
  .option('-i, --interactive', 'interactive mode', true)
  .action(async (name, options) => {
    const createApp = require('./tools/create-app.js');
    // The create-app tool handles its own argument parsing
    process.argv = ['node', 'create-app.js'];
    if (name) {
      process.argv.push('--name', name);
    }
    if (options.dir !== './apps') {
      process.argv.push('--dir', options.dir);
    }
    if (options.template !== 'basic') {
      process.argv.push('--template', options.template);
    }
    if (options.interactive) {
      process.argv.push('--interactive');
    }
  });

// Validate app command
program
  .command('validate')
  .alias('check')
  .description('Validate a T-Embed app')
  .argument('<app-path>', 'path to app directory')
  .option('-v, --verbose', 'verbose output')
  .option('-f, --fix', 'attempt to fix common issues')
  .action((appPath, options) => {
    const validateApp = require('./tools/validate-app.js');
    process.argv = ['node', 'validate-app.js', appPath];
    if (options.verbose) {
      process.argv.push('--verbose');
    }
    if (options.fix) {
      process.argv.push('--fix');
    }
  });

// Build app command
program
  .command('build')
  .description('Build a T-Embed app for deployment')
  .argument('<app-path>', 'path to app directory')
  .option('-o, --output <path>', 'output directory', './build')
  .option('-m, --minify', 'minify JavaScript code')
  .option('-c, --compress', 'create compressed archive')
  .option('-v, --verbose', 'verbose output')
  .action((appPath, options) => {
    const buildApp = require('./tools/build-app.js');
    process.argv = ['node', 'build-app.js', appPath];
    if (options.output !== './build') {
      process.argv.push('--output', options.output);
    }
    if (options.minify) {
      process.argv.push('--minify');
    }
    if (options.compress) {
      process.argv.push('--compress');
    }
    if (options.verbose) {
      process.argv.push('--verbose');
    }
  });

// Install app command
program
  .command('install')
  .alias('deploy')
  .description('Install a T-Embed app to device')
  .argument('<app-path>', 'path to built app directory')
  .option('-p, --port <port>', 'device serial port')
  .option('-i, --ip <ip>', 'device IP address')
  .option('-w, --wifi', 'install via Wi-Fi')
  .option('-f, --force', 'force installation')
  .option('-v, --verbose', 'verbose output')
  .action((appPath, options) => {
    const installApp = require('./tools/install-app.js');
    process.argv = ['node', 'install-app.js', appPath];
    if (options.port) {
      process.argv.push('--port', options.port);
    }
    if (options.ip) {
      process.argv.push('--ip', options.ip);
    }
    if (options.wifi) {
      process.argv.push('--wifi');
    }
    if (options.force) {
      process.argv.push('--force');
    }
    if (options.verbose) {
      process.argv.push('--verbose');
    }
  });

// List devices command
program
  .command('devices')
  .alias('ls')
  .description('List connected T-Embed devices')
  .option('-v, --verbose', 'verbose output')
  .action((options) => {
    console.log(chalk.blue('🔍 Scanning for T-Embed devices...'));
    
    // This would implement device discovery
    console.log(chalk.yellow('Device discovery not yet implemented'));
    console.log('Manual device detection:');
    console.log('  Serial ports: /dev/ttyUSB*, /dev/ttyACM*, COM*');
    console.log('  Network: Check device Wi-Fi AP or mDNS');
  });

// Info command
program
  .command('info')
  .description('Show SDK and device information')
  .argument('[app-path]', 'path to app directory (optional)')
  .action((appPath) => {
    console.log(chalk.blue('📋 T-Embed CC1101 SDK Information'));
    console.log('');
    console.log(`SDK Version: ${packageInfo.version}`);
    console.log(`Node Version: ${process.version}`);
    console.log(`Platform: ${process.platform}`);
    console.log('');
    
    if (appPath) {
      if (fs.existsSync(appPath)) {
        const manifestPath = path.join(appPath, 'manifest.json');
        if (fs.existsSync(manifestPath)) {
          try {
            const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
            console.log(chalk.green('📱 App Information:'));
            console.log(`  Name: ${manifest.name}`);
            console.log(`  Version: ${manifest.version}`);
            console.log(`  Description: ${manifest.description}`);
            console.log(`  Author: ${manifest.author}`);
            console.log(`  Main File: ${manifest.main}`);
            console.log(`  Permissions: ${manifest.permissions ? manifest.permissions.join(', ') : 'None'}`);
            console.log(`  Category: ${manifest.category}`);
          } catch (e) {
            console.log(chalk.red('❌ Invalid manifest.json in app directory'));
          }
        } else {
          console.log(chalk.red('❌ No manifest.json found in app directory'));
        }
      } else {
        console.log(chalk.red('❌ App directory not found'));
      }
    }
    
    console.log('');
    console.log(chalk.blue('📚 Resources:'));
    console.log('  API Reference: ./docs/API.md');
    console.log('  Development Guide: ./docs/GUIDE.md');
    console.log('  GitHub: https://github.com/your-repo/t-embed-cc1101-fw');
  });

// Examples command
program
  .command('examples')
  .description('List available example apps')
  .action(() => {
    console.log(chalk.blue('📚 Available Example Apps:'));
    console.log('');
    
    const examples = [
      {
        name: 'hello-world',
        description: 'Basic app with UI components',
        template: 'basic'
      },
      {
        name: 'rf-scanner',
        description: 'Scan for RF signals',
        template: 'rf'
      },
      {
        name: 'led-controller',
        description: 'Control GPIO pins and LEDs',
        template: 'gpio'
      },
      {
        name: 'ui-showcase',
        description: 'Demonstrate UI components',
        template: 'ui'
      }
    ];
    
    examples.forEach((example, index) => {
      console.log(chalk.green(`${index + 1}. ${example.name}`));
      console.log(`   ${example.description}`);
      console.log(chalk.gray(`   Template: ${example.template}`));
      console.log('');
    });
    
    console.log(chalk.blue('To create an example app:'));
    console.log(chalk.gray('  t-embed-sdk create my-app --template basic'));
  });

// Docs command
program
  .command('docs')
  .description('Open documentation')
  .option('-a, --api', 'open API reference')
  .option('-g, --guide', 'open development guide')
  .action((options) => {
    if (options.api) {
      const apiPath = path.join(__dirname, 'docs', 'API.md');
      console.log(chalk.blue('📖 API Reference:'), apiPath);
    } else if (options.guide) {
      const guidePath = path.join(__dirname, 'docs', 'GUIDE.md');
      console.log(chalk.blue('📘 Development Guide:'), guidePath);
    } else {
      console.log(chalk.blue('📚 Available Documentation:'));
      console.log('');
      console.log(chalk.green('API Reference:'));
      console.log('  t-embed-sdk docs --api');
      console.log('  File: ./docs/API.md');
      console.log('');
      console.log(chalk.green('Development Guide:'));
      console.log('  t-embed-sdk docs --guide');
      console.log('  File: ./docs/GUIDE.md');
      console.log('');
      console.log(chalk.blue('Online Resources:'));
      console.log('  GitHub: https://github.com/your-repo/t-embed-cc1101-fw');
      console.log('  Issues: https://github.com/your-repo/t-embed-cc1101-fw/issues');
    }
  });

// Version check command
program
  .command('update-check')
  .description('Check for SDK updates')
  .action(() => {
    console.log(chalk.blue('🔍 Checking for updates...'));
    console.log(`Current version: ${packageInfo.version}`);
    console.log(chalk.yellow('Update checking not yet implemented'));
    console.log('Check GitHub for latest releases:');
    console.log('https://github.com/your-repo/t-embed-cc1101-fw/releases');
  });

// Global error handler
program.exitOverride();

try {
  program.parse();
} catch (err) {
  if (err.code === 'commander.help') {
    process.exit(0);
  } else if (err.code === 'commander.version') {
    process.exit(0);
  } else {
    console.error(chalk.red('Error:'), err.message);
    process.exit(1);
  }
}

// Show help if no command provided
if (process.argv.length === 2) {
  console.log(chalk.blue('🚀 T-Embed CC1101 JavaScript SDK'));
  console.log('');
  console.log('Quick start:');
  console.log(chalk.green('  t-embed-sdk create my-first-app'));
  console.log(chalk.green('  t-embed-sdk validate ./apps/my-first-app'));
  console.log(chalk.green('  t-embed-sdk build ./apps/my-first-app'));
  console.log(chalk.green('  t-embed-sdk install ./build/my-first-app'));
  console.log('');
  console.log('For more commands, run:');
  console.log(chalk.gray('  t-embed-sdk --help'));
}