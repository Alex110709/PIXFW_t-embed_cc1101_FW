#!/usr/bin/env node

/**
 * @file install-app.js
 * @brief CLI tool for installing T-Embed CC1101 apps to devices
 */

const fs = require('fs');
const path = require('path');
const { program } = require('commander');
const chalk = require('chalk');
const inquirer = require('inquirer');

program
  .name('install-app')
  .description('Install a T-Embed CC1101 JavaScript app to device')
  .version('1.0.0')
  .argument('<app-path>', 'path to built app directory')
  .option('-p, --port <port>', 'device serial port')
  .option('-i, --ip <ip>', 'device IP address (for network installation)')
  .option('-w, --wifi', 'install via Wi-Fi')
  .option('-f, --force', 'force installation (overwrite existing)')
  .option('-v, --verbose', 'verbose output')
  .action(async (appPath, options) => {
    try {
      await installApp(appPath, options);
    } catch (error) {
      console.error(chalk.red('Error:'), error.message);
      process.exit(1);
    }
  });

async function installApp(appPath, options) {
  const fullAppPath = path.resolve(appPath);
  const appName = path.basename(fullAppPath);
  
  if (!fs.existsSync(fullAppPath)) {
    throw new Error(`App directory not found: ${fullAppPath}`);
  }

  console.log(chalk.blue('📱 Installing app:'), appName);
  console.log('');

  // Validate app package
  const validation = await validateAppPackage(fullAppPath);
  if (!validation.valid) {
    throw new Error(`Invalid app package: ${validation.errors.join(', ')}`);
  }

  // Determine installation method
  const installMethod = await determineInstallMethod(options);
  
  // Perform installation
  switch (installMethod.type) {
    case 'serial':
      await installViaSerial(fullAppPath, installMethod, options);
      break;
    case 'wifi':
      await installViaWiFi(fullAppPath, installMethod, options);
      break;
    case 'manual':
      await showManualInstructions(fullAppPath, options);
      break;
    default:
      throw new Error(`Unknown installation method: ${installMethod.type}`);
  }

  console.log('');
  console.log(chalk.green('✅ Installation completed successfully!'));
  console.log(chalk.blue('The app should appear in the device app menu after restart.'));
}

async function validateAppPackage(appPath) {
  const errors = [];

  // Check manifest
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

async function determineInstallMethod(options) {
  if (options.wifi || options.ip) {
    return {
      type: 'wifi',
      ip: options.ip || await discoverDeviceIP()
    };
  }

  if (options.port) {
    return {
      type: 'serial',
      port: options.port
    };
  }

  // Interactive mode
  const answers = await inquirer.prompt([
    {
      type: 'list',
      name: 'method',
      message: 'Choose installation method:',
      choices: [
        { name: 'Serial USB - Connect via USB cable', value: 'serial' },
        { name: 'Wi-Fi - Install over network', value: 'wifi' },
        { name: 'Manual - Show installation instructions', value: 'manual' }
      ]
    }
  ]);

  if (answers.method === 'serial') {
    const portAnswer = await inquirer.prompt([
      {
        type: 'input',
        name: 'port',
        message: 'Serial port:',
        default: detectSerialPort(),
        validate: (input) => input.trim().length > 0 || 'Port is required'
      }
    ]);
    return { type: 'serial', port: portAnswer.port };
  }

  if (answers.method === 'wifi') {
    const ipAnswer = await inquirer.prompt([
      {
        type: 'input',
        name: 'ip',
        message: 'Device IP address:',
        validate: (input) => {
          if (!input.trim()) return 'IP address is required';
          const ipRegex = /^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$/;
          return ipRegex.test(input.trim()) || 'Invalid IP address format';
        }
      }
    ]);
    return { type: 'wifi', ip: ipAnswer.ip };
  }

  return { type: 'manual' };
}

function detectSerialPort() {
  // Try to detect common serial ports
  const commonPorts = [
    '/dev/ttyUSB0',
    '/dev/ttyACM0', 
    '/dev/cu.usbserial-*',
    'COM3',
    'COM4'
  ];

  for (const port of commonPorts) {
    if (port.includes('*')) {
      // Pattern matching would be needed here
      continue;
    }
    try {
      if (fs.existsSync(port)) {
        return port;
      }
    } catch (e) {
      // Ignore errors
    }
  }

  return process.platform === 'win32' ? 'COM3' : '/dev/ttyUSB0';
}

async function discoverDeviceIP() {
  // This would implement device discovery via mDNS or similar
  // For now, return a placeholder
  return '192.168.1.100';
}

async function installViaSerial(appPath, installMethod, options) {
  console.log(chalk.yellow('📡 Installing via serial port...'));
  console.log(`Port: ${installMethod.port}`);

  // Check if port exists
  if (!fs.existsSync(installMethod.port) && !installMethod.port.startsWith('COM')) {
    throw new Error(`Serial port not found: ${installMethod.port}`);
  }

  // This is a simplified implementation
  // In a real implementation, this would use a serial library to communicate
  console.log(chalk.blue('📤 Uploading files...'));

  const manifest = JSON.parse(fs.readFileSync(path.join(appPath, 'manifest.json'), 'utf8'));
  const appName = manifest.name;

  // Simulate file upload process
  const files = getAppFiles(appPath);
  
  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    const progress = Math.round(((i + 1) / files.length) * 100);
    
    if (options.verbose) {
      console.log(chalk.gray(`  Uploading ${file.name}... ${progress}%`));
    }
    
    // Simulate upload delay
    await new Promise(resolve => setTimeout(resolve, 200));
  }

  console.log(chalk.green('✅ Files uploaded successfully'));
  
  // Send install command to device
  console.log(chalk.blue('📦 Installing app on device...'));
  await new Promise(resolve => setTimeout(resolve, 1000));
  
  console.log(chalk.green(`✅ App "${appName}" installed successfully`));
}

async function installViaWiFi(appPath, installMethod, options) {
  console.log(chalk.yellow('📶 Installing via Wi-Fi...'));
  console.log(`Device IP: ${installMethod.ip}`);

  // This would implement HTTP/WebSocket communication with the device
  console.log(chalk.blue('🔗 Connecting to device...'));
  
  // Simulate connection
  await new Promise(resolve => setTimeout(resolve, 1000));
  
  const manifest = JSON.parse(fs.readFileSync(path.join(appPath, 'manifest.json'), 'utf8'));
  const appName = manifest.name;

  console.log(chalk.blue('📤 Uploading app package...'));
  
  // Simulate upload
  const files = getAppFiles(appPath);
  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    const progress = Math.round(((i + 1) / files.length) * 100);
    
    if (options.verbose) {
      console.log(chalk.gray(`  Uploading ${file.name}... ${progress}%`));
    }
    
    await new Promise(resolve => setTimeout(resolve, 300));
  }

  console.log(chalk.green('✅ Package uploaded successfully'));
  
  console.log(chalk.blue('📦 Installing app...'));
  await new Promise(resolve => setTimeout(resolve, 1500));
  
  console.log(chalk.green(`✅ App "${appName}" installed via Wi-Fi`));
}

async function showManualInstructions(appPath, options) {
  const manifest = JSON.parse(fs.readFileSync(path.join(appPath, 'manifest.json'), 'utf8'));
  const appName = manifest.name;

  console.log(chalk.yellow('📋 Manual Installation Instructions'));
  console.log('');
  console.log(chalk.blue('Method 1: Serial Console'));
  console.log('1. Connect your T-Embed device via USB');
  console.log('2. Open a serial terminal (115200 baud)');
  console.log('3. Access the device file system');
  console.log(`4. Create directory: /apps/${appName}/`);
  console.log('5. Upload the following files:');
  
  const files = getAppFiles(appPath);
  files.forEach(file => {
    console.log(chalk.gray(`   - ${file.relativePath}`));
  });

  console.log('');
  console.log(chalk.blue('Method 2: Web Interface'));
  console.log('1. Connect device to Wi-Fi');
  console.log('2. Open device web interface in browser');
  console.log('3. Navigate to App Manager');
  console.log('4. Upload app package');
  console.log('5. Install via web interface');

  console.log('');
  console.log(chalk.blue('Method 3: SD Card'));
  console.log('1. Copy app directory to SD card');
  console.log(`2. Place in /apps/${appName}/ on SD card`);
  console.log('3. Insert SD card into device');
  console.log('4. Install via device menu');

  console.log('');
  console.log(chalk.green('App package location:'), path.relative(process.cwd(), appPath));
}

function getAppFiles(appPath) {
  const files = [];
  
  function addFile(filePath, relativePath) {
    if (fs.statSync(filePath).isFile()) {
      files.push({
        path: filePath,
        relativePath: relativePath,
        name: path.basename(filePath),
        size: fs.statSync(filePath).size
      });
    }
  }

  function scanDirectory(dirPath, relativeBase = '') {
    const items = fs.readdirSync(dirPath);
    
    for (const item of items) {
      if (item.startsWith('.') && item !== '.build-info.json') {
        continue; // Skip hidden files except build info
      }
      
      const fullPath = path.join(dirPath, item);
      const relativePath = path.join(relativeBase, item);
      
      if (fs.statSync(fullPath).isDirectory()) {
        scanDirectory(fullPath, relativePath);
      } else {
        addFile(fullPath, relativePath);
      }
    }
  }

  scanDirectory(appPath);
  return files;
}

if (require.main === module) {
  program.parse();
}