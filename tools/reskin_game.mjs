// Pixel-only reskin pipeline. Requires existing ImageMagick and macOS plutil.
// Run from repository root: node tools/reskin_game.mjs prepare|assemble|install
import fs from 'node:fs';
import path from 'node:path';
import {createHash} from 'node:crypto';
import {execFileSync as exec} from 'node:child_process';
const root='art/rebrand-v3';
const run=(args)=>exec('magick',args,{stdio:'pipe'}).toString();
const json=p=>JSON.parse(exec('plutil',['-convert','json','-o','-',p]).toString());
const mkdir=p=>fs.mkdirSync(p,{recursive:true});
const mode=process.argv[2];
mkdir(root);
if(mode==='prepare') {
  if(fs.existsSync(`${root}/manifest.json`)) throw Error('Baseline already exists; refusing to overwrite');
  const frames=[];
  for(const tier of ['HD','SD']) {
    mkdir(`${root}/baseline/${tier}`); mkdir(`${root}/original/${tier}`);
    for(let i=0;i<4;i++) {
      const stem=`animation_s${i}`;
      for(const ext of ['png','plist']) fs.copyFileSync(`Content/Images/${tier}/${stem}.${ext}`,`${root}/baseline/${tier}/${stem}.${ext}`);
      for(const [name,meta] of Object.entries(json(`Content/Images/${tier}/${stem}.plist`).frames)) {
        if(meta.rotated) throw Error('Rotated frame requires explicit handling');
        const [x,y,w,h]=meta.frame.match(/-?\d+/g).map(Number);
        frames.push({tier,sheet:i,name,x,y,w,h,meta});
        run([`${root}/baseline/${tier}/${stem}.png`,'-crop',`${w}x${h}+${x}+${y}`,'+repage',`${root}/original/${tier}/${name}.png`]);
      }
    }
  }
  fs.copyFileSync('Content/CompleteGameInfo.plist',`${root}/baseline/CompleteGameInfo.plist`);
  fs.writeFileSync(`${root}/manifest.json`,JSON.stringify(frames,null,2));
  console.log(`Saved baseline and extracted ${frames.length} frames`);
} else if(mode==='assemble') {
  const frames=JSON.parse(fs.readFileSync(`${root}/manifest.json`));
  const layers={0:'sky',1:'city',2:'wall',3:'trees',4:'foreground',5:'water',6:'debris',7:'reeds',8:'reeds'};
  for(const tier of ['HD','SD']) {
    mkdir(`${root}/frames/${tier}`);mkdir(`${root}/atlas/${tier}`);
    for(const f of frames.filter(f=>f.tier===tier)) {
      const original=`${root}/original/${tier}/${f.name}.png`, out=`${root}/frames/${tier}/${f.name}.png`;
      const bg=f.name.match(/^BackgroundLayers_(\d)_0$/);
      if(bg) {
        const n=Number(bg[1]), source=`${root}/sources/${layers[n]}.png`;
        let args=[source];
        if([1,3,4,6,7,8].includes(n)) {
          // Remove generator padding; keep the full canvas size dictated by the atlas.
          const geom=run([source,'-alpha','extract','-threshold','35%','-format','%@','info:']).trim();
          args.push('-crop',geom,'+repage');
        }
        args.push('-resize',`${f.w}x${f.h}!`);
        if(n===3) args.push('-modulate','63,75,100');
        if(n===4) args.push('-modulate','64,80,100');
        if(n===7) args.push('-flop','-modulate','75,80,100');
        if(n===8) args.push('-modulate','88,80,100');
        if(n===2) args.push('-modulate','72,78,100');
        args.push('-depth','8',out);run(args);
      } else if(f.name==='InterfaceMM_1_0') {
        const source=`${root}/sources/logo.png`;
        const geom=run([source,'-alpha','extract','-threshold','35%','-format','%@','info:']).trim();
        run([source,'-crop',geom,'+repage','-resize',`${f.w-12}x${f.h-12}`,'-gravity','center','-background','none','-extent',`${f.w}x${f.h}`,'-depth','8',out]);
      } else if(['InterfaceMM_0_0','InterfaceCS_0_0','InterfaceCoinShop_0_0'].includes(f.name)) {
        run([`${root}/sources/menu.png`,'-resize',`${f.w}x${f.h}^`,'-gravity','center','-extent',`${f.w}x${f.h}`,'-modulate',f.name==='InterfaceMM_0_0'?'85,90,100':'60,75,100','-depth','8',out]);
      } else {
        let ops=[];
        // Consistent treatment across every animation pose, retaining all original RGB details.
        if(f.name.startsWith('Characters')) ops=['-modulate','109,86,100','-unsharp','0x0.65+0.55+0.02'];
        else if(f.name.startsWith('Obstacles')) ops=['-modulate','112,70,100','-unsharp','0x0.7+0.65+0.03'];
        else if(f.name.startsWith('Coins')) ops=['-modulate','110,83,100','-unsharp','0x0.5+0.45+0.02'];
        else if(f.name.startsWith('PZGameplay')) ops=['-modulate','98,85,100'];
        else if(f.name.startsWith('Interface')) ops=['-modulate','102,65,100','-unsharp','0x0.5+0.3+0.02'];
        run([original,'-channel','RGB',...ops,'+channel',out]);
        // Restore byte-exact original alpha after every color/material operation.
        run([original,'-alpha','extract',`${root}/alpha.png`]);
        run([out,'-alpha','off',`${root}/alpha.png`,'-alpha','off','-compose','CopyOpacity','-composite','-depth','8',out]);
      }
    }
    for(let i=0;i<4;i++) {
      const args=[`${root}/baseline/${tier}/animation_s${i}.png`];
      for(const f of frames.filter(f=>f.tier===tier&&f.sheet===i)) args.push(`${root}/frames/${tier}/${f.name}.png`,'-geometry',`+${f.x}+${f.y}`,'-compose','Copy','-composite');
      args.push('-depth','8',`${root}/atlas/${tier}/animation_s${i}.png`);run(args);
    }
  }
  console.log('Built candidate atlases; run validation before install');
} else if(mode==='install') {
  if(!fs.existsSync(`${root}/validation.json`)) throw Error('Run validation first');
  const report=JSON.parse(fs.readFileSync(`${root}/validation.json`));
  for(const tier of ['HD','SD']) for(let i=0;i<4;i++) {
    const bytes=fs.readFileSync(`${root}/atlas/${tier}/animation_s${i}.png`);
    if(createHash('sha256').update(bytes).digest('hex')!==report.atlases[`${tier}/animation_s${i}`]) throw Error('Candidate changed since validation');
    if(!fs.readFileSync(`Content/Images/${tier}/animation_s${i}.plist`).equals(fs.readFileSync(`${root}/baseline/${tier}/animation_s${i}.plist`))) throw Error('Atlas metadata changed since baseline');
  }
  for(const tier of ['HD','SD']) for(let i=0;i<4;i++) fs.copyFileSync(`${root}/atlas/${tier}/animation_s${i}.png`,`Content/Images/${tier}/animation_s${i}.png`);
  console.log('Installed pixel-only atlas replacements');
} else throw Error('Expected prepare, assemble, or install');
