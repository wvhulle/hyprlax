#!/usr/bin/env node

import sharp from 'sharp';
import { readFileSync, writeFileSync, mkdirSync } from 'fs';
import { join } from 'path';

const SIZES = [
  // Standard favicons
  { size: 16, name: 'favicon-16x16.png' },
  { size: 32, name: 'favicon-32x32.png' },
  { size: 48, name: 'favicon-48x48.png' },
  { size: 64, name: 'favicon-64x64.png' },
  { size: 96, name: 'favicon-96x96.png' },
  { size: 128, name: 'favicon-128x128.png' },
  { size: 256, name: 'favicon-256x256.png' },
  
  // Apple Touch Icons
  { size: 180, name: 'apple-touch-icon.png' },
  { size: 152, name: 'apple-touch-icon-152x152.png' },
  { size: 144, name: 'apple-touch-icon-144x144.png' },
  { size: 120, name: 'apple-touch-icon-120x120.png' },
  { size: 114, name: 'apple-touch-icon-114x114.png' },
  { size: 76, name: 'apple-touch-icon-76x76.png' },
  { size: 72, name: 'apple-touch-icon-72x72.png' },
  { size: 60, name: 'apple-touch-icon-60x60.png' },
  { size: 57, name: 'apple-touch-icon-57x57.png' },
  
  // Android/PWA
  { size: 512, name: 'android-chrome-512x512.png' },
  { size: 384, name: 'android-chrome-384x384.png' },
  { size: 256, name: 'android-chrome-256x256.png' },
  { size: 192, name: 'android-chrome-192x192.png' },
  
  // Microsoft Tiles
  { size: 144, name: 'mstile-144x144.png' },
  { size: 150, name: 'mstile-150x150.png' },
];

async function generateFavicons() {
  console.log('🎨 Generating favicons from hyprlax-icon.svg...\n');
  
  const svgBuffer = readFileSync('public/hyprlax-icon.svg');
  
  // Generate all PNG sizes
  for (const { size, name } of SIZES) {
    await sharp(svgBuffer, { density: 300 })
      .resize(size, size)
      .png()
      .toFile(join('public', name));
    
    console.log(`✓ ${name} (${size}x${size})`);
  }
  
  // Generate ICO with multiple sizes
  console.log('\n📦 Generating favicon.ico...');
  await sharp(svgBuffer)
    .resize(32, 32)
    .toFile('public/favicon.ico');
  console.log('✓ favicon.ico');
  
  // Generate Web App Manifest
  console.log('\n📱 Creating manifest.json...');
  const manifest = {
    name: 'hyprlax',
    short_name: 'hyprlax',
    description: 'Smooth parallax wallpaper animations for Hyprland',
    theme_color: '#050810',
    background_color: '#050810',
    display: 'standalone',
    scope: '.',
    start_url: '.',
    icons: [
      { src: 'android-chrome-192x192.png', sizes: '192x192', type: 'image/png' },
      { src: 'android-chrome-256x256.png', sizes: '256x256', type: 'image/png' },
      { src: 'android-chrome-384x384.png', sizes: '384x384', type: 'image/png' },
      { src: 'android-chrome-512x512.png', sizes: '512x512', type: 'image/png', purpose: 'any maskable' }
    ]
  };
  
  writeFileSync('public/manifest.json', JSON.stringify(manifest, null, 2));
  console.log('✓ manifest.json');
  
  // Generate browserconfig for MS Edge/IE
  console.log('\n🪟 Creating browserconfig.xml...');
  const browserconfig = `<?xml version="1.0" encoding="utf-8"?>
<browserconfig>
  <msapplication>
    <tile>
      <square150x150logo src="mstile-150x150.png"/>
      <TileColor>#050810</TileColor>
    </tile>
  </msapplication>
</browserconfig>`;
  
  writeFileSync('public/browserconfig.xml', browserconfig);
  console.log('✓ browserconfig.xml');
  
  console.log('\n✨ All favicons generated successfully!');
  console.log('📸 Largest PNG: android-chrome-512x512.png (512x512)');
}

// Run the script
generateFavicons().catch(err => {
  console.error('❌ Error generating favicons:', err);
  process.exit(1);
});
