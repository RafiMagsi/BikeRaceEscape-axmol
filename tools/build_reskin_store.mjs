// Build promotional art and screenshot layouts with ImageMagick, preserving capture aspect ratios.
import fs from 'node:fs';
import path from 'node:path';
import {execFileSync as exec} from 'node:child_process';
const src='art/rebrand-v3/sources', out='store/rebrand-v3';
fs.mkdirSync(`${out}/work`,{recursive:true});
fs.mkdirSync(`${out}/listing`,{recursive:true});
const magick=a=>exec('magick',a,{stdio:'pipe'}).toString();
const font='/System/Library/Fonts/Supplemental/Arial Bold.ttf';
const regular='/System/Library/Fonts/Supplemental/Arial.ttf';
function cover(source,w,h,dest) {magick([source,'-resize',`${w}x${h}^`,'-gravity','center','-extent',`${w}x${h}`,'-alpha','off',dest]);}
function title(args,text,x,y,size,color='#f5f0df',face=font) {args.push('-font',face,'-pointsize',String(size),'-fill',color,'-gravity','NorthWest','-annotate',`+${x}+${y}`,text);}
function promotional(w,h,dest){
  const temp=`${out}/work/hero-${w}-${h}.png`;cover(`${src}/hero.png`,w,h,temp);
  const logo=`${out}/work/logo-${w}.png`;
  magick(['art/rebrand-v3/frames/HD/InterfaceMM_1_0.png','-resize',`${Math.round(w*.35)}x${Math.round(h*.55)}`,logo]);
  const args=[temp,logo,'-geometry',`+${Math.round(w*.035)}+${Math.round(h*.16)}`,'-compose','Over','-composite'];
  title(args,'RIDE. JUMP. ESCAPE.',Math.round(w*.05),Math.round(h*.73),Math.round(h*.035));
  args.push('-alpha','off',dest);magick(args);
}
promotional(1920,1080,`${out}/header-1920x1080.png`);
promotional(3840,1646,`${out}/header-3840x1646.png`);
promotional(1024,500,`${out}/feature-graphic-1024x500.png`);
for(const size of [1024,512])cover(`${src}/icon.png`,size,size,`${out}/icon-${size}.png`);
for(const [file,source] of [
 ['store/ios/app_store_header_1920x1080.png',`${out}/header-1920x1080.png`],
 ['store/ios/app_store_header_3840x1646.png',`${out}/header-3840x1646.png`],
 ['store/android/feature_graphic_1024x500.png',`${out}/feature-graphic-1024x500.png`],
 ['store/icon/icon_1024.png',`${out}/icon-1024.png`],
 ['store/icon/play_icon_512.png',`${out}/icon-512.png`],
 ['store/ios/app_store_search_result_1024.png',`${out}/icon-1024.png`],
 ['store/android/app_store_search_result_1024.png',`${out}/icon-1024.png`],
 ['proj.android/app/res/play-store/ic_launcher_512.png',`${out}/icon-512.png`]
])fs.copyFileSync(source,file);
magick([`${out}/header-3840x1646.png`,'-quality','94','store/ios/app_store_header_3840x1646.jpg']);
// Keep every existing app-icon filename and dimension intact.
const ios='proj.ios_mac/ios/targets/ios/Images.xcassets/AppIcon.appiconset';
for(const dir of [ios,...['mdpi','hdpi','xhdpi','xxhdpi','xxxhdpi'].map(d=>`proj.android/app/res/mipmap-${d}`)]) {
  for(const file of fs.readdirSync(dir).filter(f=>f.endsWith('.png'))) {
    const dest=path.join(dir,file),size=magick(['identify','-format','%wx%h',dest]);
    magick([`${src}/icon.png`,'-resize',`${size}!`,'-alpha','off',dest]);
  }
}
// Captures are unaltered simulator renders, not reconstructed gameplay.
const manifestPath='art/rebrand-v3/captures/selected.json';
if(fs.existsSync(manifestPath)) {
  const captures=JSON.parse(fs.readFileSync(manifestPath));
  const formats=[['ios/iphone_6.9',2868,1320],['ios/iphone_6.3',2622,1206],['ios/ipad_13',2752,2064],['android/phone',1920,1080],['android/tablet_7',1920,1080],['android/tablet_10',2560,1440],['rebrand-v3/listing',1920,1080]];
  const spec=[['01_escape','ENTER THE SEWER RUN','A new look. The same escape.',captures.menu],['02_jump','MAKE THE JUMP','Time your takeoff. Find the next landing.',captures.jump],['03_coins','KEEP THE RUN ALIVE','Collect coins along the route.',captures.run],['04_bikes','CHOOSE YOUR RIDE','Find your next bike.',captures.bikes],['05_sewer','WATCH WHAT LURKS','Stay sharp through the drainage district.',captures.sewer]];
  for(const [dir,w,h] of formats) {
    fs.mkdirSync(`store/${dir}`,{recursive:true});fs.mkdirSync(`store/${dir}_jpg`,{recursive:true});
    for(const [name,heading,sub,capture] of spec) {
      const margin=Math.round(w*.035), top=Math.round(h*.20),availH=h-top-margin;
      const shot=`${out}/work/capture-${w}-${h}.png`;
      magick([capture,'-resize',`${w-margin*2}x${availH}`,shot]);
      const [sw,sh]=magick(['identify','-format','%w %h',shot]).split(' ').map(Number);
      const args=['-size',`${w}x${h}`,'xc:#0b1720',shot,'-geometry',`+${Math.round((w-sw)/2)}+${top+Math.round((availH-sh)/2)}`,'-compose','Over','-composite'];
      title(args,heading,margin,Math.round(h*.035),Math.round(h*.063));
      title(args,sub,margin,Math.round(h*.12),Math.round(h*.026),'#b6c5ca',regular);
      title(args,'DINO BIKE ESCAPE',Math.round(w*.75),Math.round(h*.14),Math.round(h*.018),'#e4ac5c');
      const dest=`store/${dir}/${name}.png`;args.push('-alpha','off',dest);magick(args);
      magick([dest,'-quality','94',`store/${dir}_jpg/${name}.jpg`]);
    }
  }
}
console.log('Store artwork and installed app icons exported');
