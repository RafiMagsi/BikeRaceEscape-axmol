#!/usr/bin/env python3
# Synthesises the game's sound effects (numpy) and voice lines (macOS `say` + ffmpeg grading) into Content/Sounds.
import numpy as np, wave, subprocess, os, sys
SR=22050
OUT='/Users/rafi/developer/games/remake/BikeRaceEscape-axmol/Content/Sounds'
TMP='/private/tmp/claude-501/-Users-rafi-developer-games-remake-BikeRaceEscape-axmol/e3eb76ea-655c-414a-801f-6474ebefe9a1/scratchpad/audio'
rng=np.random.default_rng(7)
def t(d): return np.linspace(0,d,int(SR*d),endpoint=False)
def env(n,a=0.005,d=0.0,power=2.0):
    x=np.linspace(0,1,n); e=(1-x)**power
    na=max(1,int(a*SR)); e[:na]*=np.linspace(0,1,na); return e
def sine(f,d,ph=0): return np.sin(2*np.pi*f*t(d)+ph)
def sweep(f0,f1,d,expo=False):
    tt=t(d); 
    f=np.geomspace(f0,f1,len(tt)) if expo else np.linspace(f0,f1,len(tt))
    return np.sin(2*np.pi*np.cumsum(f)/SR)
def noise(d): return rng.uniform(-1,1,int(SR*d))
def lp(x,fc):
    a=np.exp(-2*np.pi*fc/SR); y=np.zeros_like(x); s=0.0
    for i,v in enumerate(x): s=(1-a)*v+a*s; y[i]=s
    return y
def hp(x,fc): return x-lp(x,fc)
def norm(x,peak=0.9): 
    m=np.max(np.abs(x))+1e-9; return x/m*peak
def tri(d,peak):
    n=int(SR*d); k=max(1,int(n*peak)); return np.concatenate([np.linspace(0,1,k),np.linspace(1,0,n-k)])
def msum(g): return mix(*list(g))
def mix(*parts):
    n=max(len(p) for p in parts); y=np.zeros(n)
    for p in parts: y[:len(p)]+=p
    return y
def at(x,off,n=None):
    pad=np.zeros(int(off*SR)); y=np.concatenate([pad,x]); return y
def save(name,x,peak=0.9):
    x=norm(x,peak); path=f'{TMP}/{name}.wav'
    with wave.open(path,'wb') as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR); w.writeframes((x*32767).astype(np.int16).tobytes())
    subprocess.run(['ffmpeg','-y','-loglevel','error','-i',path,'-ac','1','-ar',str(SR),'-b:a','56k',f'{OUT}/{name}.mp3'],check=True)
def tone(f,d,a=0.004,power=2.5,harm=(1,0.4,0.2)):
    y=sum(h*np.sin(2*np.pi*f*(i+1)*t(d)) for i,h in enumerate(harm)); return y*env(len(y),a,power=power)
def chime(f,d=0.5): return tone(f,d,0.002,3.0,(1,0.5,0.25,0.12))
# ---------- effects ----------
fx={}
fx['fx_jump']=sweep(260,720,0.16)*env(int(SR*0.16),0.01,power=1.5)*0.6+hp(noise(0.16),2000)*env(int(SR*0.16),0.01,power=2)*0.15
fx['fx_land']=mix(sweep(140,50,0.18,True)*env(int(SR*0.18),0.002,power=2.5), lp(noise(0.12),900)*env(int(SR*0.12),0.001,power=3)*0.7)
fx['fx_powerup']=msum(at(chime(f,0.35),i*0.07) for i,f in enumerate([523,659,784,1047]))
fx['fx_shield']=mix(sweep(300,900,0.3)*env(int(SR*0.3),0.02,power=1.2)*0.5, at(chime(1318,0.4),0.15))
fx['fx_shield_break']=mix(hp(noise(0.35),1500)*env(int(SR*0.35),0.001,power=3), sweep(1400,200,0.3,True)*env(int(SR*0.3),0.001,power=2)*0.7)
fx['fx_magnet']=mix(sweep(200,700,0.45,True)*env(int(SR*0.45),0.02,power=1.0)*0.5, at(chime(988,0.3),0.2), at(chime(1480,0.3),0.3))
fx['fx_turbo']=mix(lp(noise(0.8),2500)*np.linspace(0.1,1,int(SR*0.8))*env(int(SR*0.8),0.05,power=0.6)*0.6, sweep(90,380,0.8)*np.linspace(0.2,1,int(SR*0.8))*0.5)
fx['fx_jetpack']=mix(lp(noise(0.7),1800)*env(int(SR*0.7),0.01,power=1.2)*0.8, sweep(120,220,0.7)*env(int(SR*0.7),0.02,power=1.2)*0.4)
fx['fx_rocket']=mix(lp(noise(0.9),3000)*tri(0.9,0.17)*0.9, sweep(70,260,0.9)*env(int(SR*0.9),0.01,power=1.0)*0.5)
fx['fx_gun_on']=mix(tone(180,0.08,0.001,2,(1,0.6))*0.8, at(tone(320,0.1,0.001,2,(1,0.5)),0.09)*0.8, at(hp(noise(0.05),3000)*env(int(SR*0.05),0.001),0.19)*0.5)
fx['fx_shot']=mix(sweep(900,120,0.14,True)*env(int(SR*0.14),0.001,power=2.5), hp(noise(0.08),1200)*env(int(SR*0.08),0.001,power=3)*0.7)
fx['fx_blast']=mix(sweep(120,28,1.4,True)*env(int(SR*1.4),0.003,power=1.6)*1.0, lp(noise(1.2),1400)*env(int(SR*1.2),0.002,power=2.2)*0.9, at(sweep(2000,300,0.5,True)*env(int(SR*0.5),0.001,power=2)*0.3,0.0))
def boom(f0,d,nlp): return mix(sweep(f0,40,d,True)*env(int(SR*d),0.002,power=2.0), lp(noise(d),nlp)*env(int(SR*d),0.001,power=2.5)*0.9)
fx['fx_explosion']=boom(200,0.55,1800)
fx['fx_smash']=mix(hp(noise(0.22),500)*env(int(SR*0.22),0.001,power=3.5)*0.9, sweep(300,80,0.14,True)*env(int(SR*0.14),0.001,power=2.5))
fx['fx_crash']=mix(boom(180,0.7,1200), at(hp(noise(0.25),800)*env(int(SR*0.25),0.001,power=3)*0.6,0.03))
fx['fx_fall']=sweep(900,160,0.7,True)*env(int(SR*0.7),0.01,power=1.2)*0.6
fx['fx_combo']=chime(880,0.18)
fx['fx_combo_bonus']=msum(at(chime(f,0.25),i*0.06) for i,f in enumerate([784,988,1318]))
fx['fx_nearmiss']=mix(hp(lp(noise(0.3),5000),800)*tri(0.3,0.4)*0.7, sweep(500,1200,0.3)*0.15)
fx['fx_bigair']=mix(msum(at(chime(f,0.3),i*0.05) for i,f in enumerate([784,988,1175,1568,1976])), sweep(300,1500,0.3)*0.15)
fx['fx_stage']=mix(*[at(tone(f,0.28,0.005,1.5,(1,0.5,0.3,0.1)),i*0.12) for i,f in enumerate([392,494,587,784])], at(tone(784,0.5,0.005,1.5,(1,0.5,0.3)),0.5)*0.9)
fx['fx_revive']=mix(sweep(200,900,0.6)*env(int(SR*0.6),0.05,power=0.8)*0.5, at(chime(784,0.5),0.3), at(chime(1047,0.6),0.45))
fx['fx_tick']=tone(660,0.09,0.001,3.0,(1,0.3))
fx['fx_go_hit']=mix(tone(880,0.4,0.002,2.5,(1,0.5,0.3)), tone(1318,0.4,0.002,2.5,(1,0.4)))
fx['fx_levelup']=mix(*[at(tone(f,0.3,0.005,1.4,(1,0.5,0.25)),i*0.1) for i,f in enumerate([523,659,784,1047,1318])])
fx['fx_mission']=mix(at(chime(988,0.35),0), at(chime(1318,0.5),0.12))
fx['fx_open']=mix(hp(noise(0.2),1000)*tri(0.2,0.4)*0.3, sweep(400,800,0.18)*env(int(SR*0.18),0.01,power=1.5)*0.3)
fx['fx_buy']=mix(at(chime(1568,0.15),0), at(chime(2093,0.35),0.09), hp(noise(0.05),4000)*env(int(SR*0.05),0.001)*0.3)
fx['fx_error']=mix(tone(150,0.25,0.002,1.5,(1,0.7,0.5)),tone(160,0.25,0.002,1.5,(1,0.7,0.5)))*0.9
fx['fx_claim']=msum(at(chime(f,0.4),i*0.08) for i,f in enumerate([659,880,1175,1568]))
fx['fx_helmet']=mix(tone(520,0.3,0.001,3.5,(1,0.7,0.5,0.4)), hp(noise(0.06),2000)*env(int(SR*0.06),0.001)*0.5)
fx['fx_swoosh']=mix(hp(noise(0.3),1500)*tri(0.3,0.33)*0.5, sweep(300,900,0.3)*0.1)
# legacy indices the game already references but never shipped: 4 = shot, 5/6/7 = obstacle deaths
legacy={'Sounds_4':fx['fx_shot'],'Sounds_5':boom(220,0.5,2200),'Sounds_6':mix(boom(160,0.55,1500),hp(noise(0.2),900)*env(int(SR*0.2),0.001,power=3)*0.4),'Sounds_7':boom(260,0.45,2800)}
if len(sys.argv)<2 or sys.argv[1]!='voice':
    for k,v in fx.items(): save(k,v)
    for k,v in legacy.items(): save(k,v)
# ---------- voice ----------
VOICE='Ralph'
lines={'vo_go':'Go!','vo_second_chance':'Second chance!','vo_get_ready':'Get ready!','vo_shield':'Shield!','vo_blast':'Blast!',
 'vo_jetpack':'Jetpack!','vo_rocket':'Rocket bike!','vo_gun':'Gun mode!','vo_turbo':'Turbo!','vo_magnet':'Magnet!','vo_helmet':'Lucky helmet!',
 'vo_new_best':'New best!','vo_stage':'Stage up!','vo_mission':'Mission complete!','vo_big_air':'Big air!','vo_close_call':'Close call!',
 'vo_smash':'Smash!','vo_welcome':'Welcome back!','vo_revived':"Let's go!",'vo_3':'Three','vo_2':'Two','vo_1':'One','vo_combo':'Combo!','vo_game_over':'Game over'}
for name,text in lines.items():
    aiff=f'{TMP}/{name}.aiff'
    subprocess.run(['say','-v',VOICE,'-r','150','-o',aiff,text],check=True)
    # deep trailer-voice grade: pitch down ~12%, low shelf boost, compress, slight slap echo
    af='asetrate=22050*0.88,aresample=22050,atempo=1.1,equalizer=f=140:t=q:w=1.0:g=6,equalizer=f=3200:t=q:w=1.5:g=3,acompressor=threshold=-18dB:ratio=4:makeup=6,aecho=0.6:0.5:70:0.25,alimiter=limit=0.9'
    subprocess.run(['ffmpeg','-y','-loglevel','error','-i',aiff,'-af',af,'-ac','1','-ar','22050','-b:a','56k',f'{OUT}/{name}.mp3'],check=True)
print('done')
