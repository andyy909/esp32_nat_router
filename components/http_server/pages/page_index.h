/* Index page templates - Andi Edition live dashboard */
#include "router_config.h"

#if CONFIG_ETH_UPLINK
#define INDEX_TITLE "ESP32 NAT Router (LAN)"
#else
#define INDEX_TITLE "ESP32 NAT Router"
#endif

/* Index Page - Chunked for streaming */
#define INDEX_CHUNK_HEAD "<html>\
<head>\
<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0'>\
<meta charset='UTF-8'>\
<title>" INDEX_TITLE "</title>\
<link rel='icon' href='favicon.png'>\
<style>\
*{box-sizing:border-box;margin:0;padding:0}\
:root{--cyan:#00d9ff;--green:#4caf50;--yellow:#ffc107;--red:#ff5252;--muted:#8c96a8;--panel:rgba(30,30,46,.94);--card:rgba(22,33,62,.72)}\
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'Helvetica Neue',Arial,sans-serif;background:linear-gradient(135deg,#111827 0%,#16213e 55%,#111827 100%);color:#e5e7eb;padding:1rem;min-height:100vh;line-height:1.5}\
#container{max-width:880px;margin:0 auto;padding:1.25rem;background:var(--panel);border-radius:18px;box-shadow:0 12px 40px rgba(0,0,0,.45);border:1px solid rgba(0,217,255,.09)}\
.header{display:flex;justify-content:space-between;align-items:center;gap:.75rem;margin-bottom:.8rem}\
.brand{display:flex;align-items:center;gap:.8rem;min-width:0}\
.brand img{width:58px;height:58px;border:none;flex:0 0 auto}\
h1{font-size:1.55rem;font-weight:650;color:var(--cyan);line-height:1.15;text-shadow:0 0 18px rgba(0,217,255,.25)}\
.edition{display:inline-block;margin-top:.28rem;padding:.15rem .48rem;border-radius:999px;border:1px solid rgba(0,217,255,.28);background:rgba(0,217,255,.08);color:#a8efff;font-size:.68rem;font-weight:700;letter-spacing:.06em;text-transform:uppercase}\
h2{font-size:1.08rem;font-weight:600;color:var(--cyan);margin:1.25rem 0 .7rem}\
.livebar{display:flex;align-items:center;justify-content:space-between;gap:.75rem;margin:.2rem 0 .8rem;color:var(--muted);font-size:.78rem}\
.live-state{display:flex;align-items:center;gap:.45rem}\
.live-dot{width:9px;height:9px;border-radius:50%;background:var(--green);box-shadow:0 0 10px rgba(76,175,80,.75)}\
.live-dot.offline{background:var(--red);box-shadow:0 0 10px rgba(255,82,82,.65)}\
.refresh-button{appearance:none;border:1px solid rgba(0,217,255,.25);background:rgba(0,217,255,.08);color:#b9f4ff;border-radius:8px;padding:.42rem .68rem;font-size:.76rem;cursor:pointer}\
.refresh-button:active{transform:translateY(1px)}\
.warning-box{display:none;margin:.65rem 0;padding:.8rem .9rem;border-radius:10px;border:1px solid rgba(255,193,7,.5);background:rgba(255,193,7,.1);color:#ffe082;font-size:.84rem}\
.warning-box.show{display:block}\
.cards{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:.75rem}\
.card{min-height:112px;padding:.9rem;background:var(--card);border:1px solid rgba(0,217,255,.11);border-radius:13px;box-shadow:inset 0 1px 0 rgba(255,255,255,.025)}\
.card-label{font-size:.72rem;text-transform:uppercase;letter-spacing:.055em;color:var(--muted);font-weight:700}\
.card-value{margin-top:.32rem;font-size:1.23rem;font-weight:720;color:#f3f4f6;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}\
.card-sub{margin-top:.27rem;font-size:.76rem;color:#aab3c2;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}\
.good{color:#81c784!important}.warn{color:#ffd54f!important}.bad{color:#ff8a80!important}\
.signal{display:flex;gap:3px;align-items:flex-end;height:20px;margin-top:.42rem}\
.signal i{display:block;width:5px;border-radius:2px;background:#354157}.signal i:nth-child(1){height:5px}.signal i:nth-child(2){height:9px}.signal i:nth-child(3){height:13px}.signal i:nth-child(4){height:17px}.signal i.on{background:var(--cyan);box-shadow:0 0 5px rgba(0,217,255,.45)}\
.details-card,.clients-card{margin-top:1rem;background:rgba(22,33,62,.5);border:1px solid rgba(0,217,255,.1);border-radius:13px;overflow:hidden}\
.details-card summary{cursor:pointer;padding:.85rem 1rem;color:#bcefff;font-size:.88rem;font-weight:600;list-style:none}.details-card summary::-webkit-details-marker{display:none}\
.details-card summary:after{content:'+';float:right;color:var(--cyan);font-size:1.05rem}.details-card[open] summary:after{content:'−'}\
.status-table{padding:0 .8rem .8rem}\
.status-table table{width:100%;border-collapse:collapse}\
.status-table td{padding:.58rem .45rem;font-size:.84rem;border-bottom:1px solid rgba(255,255,255,.045)}\
.status-table tr:last-child td{border-bottom:none}\
.status-table td:first-child{color:var(--muted);text-align:right;padding-right:.85rem;width:42%;font-size:.79rem}\
.status-table td:last-child{font-weight:500;overflow-wrap:anywhere}\
.clients-head{display:flex;align-items:center;justify-content:space-between;gap:.6rem;padding:.85rem 1rem;border-bottom:1px solid rgba(255,255,255,.05)}\
.clients-title{color:#bcefff;font-size:.9rem;font-weight:650}.clients-note{color:var(--muted);font-size:.72rem}\
.client-wrap{overflow-x:auto}\
.client-table{width:100%;border-collapse:collapse;min-width:560px}\
.client-table th{padding:.62rem .7rem;text-align:left;color:var(--cyan);font-size:.72rem;text-transform:uppercase;letter-spacing:.035em;background:rgba(0,217,255,.055)}\
.client-table td{padding:.7rem;font-size:.8rem;border-bottom:1px solid rgba(255,255,255,.045);vertical-align:middle}\
.client-table tbody tr:last-child td{border-bottom:none}\
.client-empty{text-align:center!important;color:var(--muted);padding:1.1rem!important}\
.device-name{font-weight:650;color:#edf2f7}.device-mac{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;color:#aab3c2;font-size:.73rem}\
.manage-link{display:inline-block;color:#bcefff;text-decoration:none;border:1px solid rgba(0,217,255,.22);background:rgba(0,217,255,.07);padding:.28rem .5rem;border-radius:6px;font-size:.72rem}\
.button-container{display:grid;grid-template-columns:repeat(3,1fr);gap:.72rem;margin:1.15rem 0 .35rem}\
.nav-button{background:linear-gradient(135deg,rgba(102,126,234,.92),rgba(118,75,162,.92));color:#fff;border:none;border-radius:11px;padding:.92rem .7rem;font-size:.86rem;font-weight:650;text-decoration:none;display:flex;align-items:center;justify-content:center;text-align:center;box-shadow:0 4px 14px rgba(102,126,234,.25)}\
.nav-button:active{transform:translateY(1px)}\
@media(max-width:720px){body{padding:.45rem}#container{padding:.85rem;border-radius:13px}.brand img{width:48px;height:48px}h1{font-size:1.25rem}.cards{grid-template-columns:repeat(2,minmax(0,1fr));gap:.58rem}.card{min-height:103px;padding:.75rem}.card-value{font-size:1.05rem}.button-container{grid-template-columns:repeat(2,1fr)}.status-table td:first-child{width:46%}}\
@media(max-width:380px){.cards{grid-template-columns:1fr}.button-container{grid-template-columns:1fr}}\
</style>\
<script>\
(function(){\
var lastBytes=null,lastTime=0,statusTimer=null,clientsTimer=null;\
function text(el){return el?el.textContent.replace(/^\\s+|\\s+$/g,''):'';}\
function setText(id,value){var el=document.getElementById(id);if(el)el.textContent=value;}\
function setClass(id,state){var el=document.getElementById(id);if(!el)return;el.className='card-value '+(state||'');}\
function rowsMap(doc){var map={},rows=doc.querySelectorAll('.status-table tr');for(var i=0;i<rows.length;i++){var c=rows[i].querySelectorAll('td');if(c.length>1){var k=text(c[0]).replace(/:$/,'');map[k]=text(c[1]);}}return map;}\
function unitBytes(n,u){var m={B:1,KB:1024,MB:1048576,GB:1073741824,TB:1099511627776};return n*(m[u]||1);}\
function parseBytes(s){var r=/([0-9.]+)\\s*(B|KB|MB|GB|TB)\\s*sent\\s*\\/\\s*([0-9.]+)\\s*(B|KB|MB|GB|TB)\\s*received/i.exec(s||'');return r?{tx:unitBytes(parseFloat(r[1]),r[2].toUpperCase()),rx:unitBytes(parseFloat(r[3]),r[4].toUpperCase())}:null;}\
function speed(v){if(!isFinite(v)||v<0)return '—';var u=['B/s','KB/s','MB/s','GB/s'],i=0;while(v>=1024&&i<u.length-1){v/=1024;i++;}return (v>=100||i===0?v.toFixed(0):v>=10?v.toFixed(1):v.toFixed(2))+' '+u[i];}\
function humanBytes(v){if(!isFinite(v)||v<0)return '—';var u=['B','KB','MB','GB','TB'],i=0;while(v>=1024&&i<u.length-1){v/=1024;i++;}return (v>=100||i===0?v.toFixed(0):v>=10?v.toFixed(1):v.toFixed(2))+' '+u[i];}\
function signalQuality(rssi){return Math.max(0,Math.min(100,2*(rssi+100)));}\
function signalBars(q){var bars=document.querySelectorAll('#signal-bars i'),on=q>=80?4:q>=60?3:q>=35?2:q>0?1:0;for(var i=0;i<bars.length;i++)bars[i].className=i<on?'on':'';}\
function pick(map,names){for(var i=0;i<names.length;i++)if(map[names[i]]!==undefined)return map[names[i]];return '';}\
function updateFromDoc(doc){\
var map=rowsMap(doc),uplink=pick(map,['Uplink']),clients=pick(map,['AP Clients']),uptime=pick(map,['Uptime']),ip=pick(map,['STA IP','ETH IP']),bytes=pick(map,['Bytes']);\
var connected=/connected/i.test(uplink)&&!/disconnected/i.test(uplink),r=/(-?\\d+)\\s*dBm/i.exec(uplink),rssi=r?parseInt(r[1],10):null,q=rssi===null?0:signalQuality(rssi);\
setText('live-uplink',connected?'Online':'Offline');setClass('live-uplink',connected?'good':'bad');setText('live-uplink-sub',connected?(ip&&ip!=='N/A'?ip:'Verbindung steht'):'Kein Internet-Uplink');\
setText('live-clients',clients||'0');setText('live-clients-sub',(clients==='1'?'Gerät verbunden':'Geräte verbunden'));\
setText('live-uptime',uptime?uptime.split(' (')[0]:'—');setText('live-uptime-sub',uptime.indexOf('since ')>=0?uptime.split('since ')[1].replace(')',''):'seit dem letzten Start');\
setText('live-signal',rssi===null?'—':rssi+' dBm');setClass('live-signal',rssi===null?'':rssi<-78?'bad':rssi<-68?'warn':'good');setText('live-signal-sub',rssi===null?'keine Messung':Math.round(q)+' % Qualität');signalBars(q);\
var b=parseBytes(bytes),now=Date.now();if(b){setText('live-total',humanBytes(b.rx));setText('live-total-sub','Upload '+humanBytes(b.tx));}else{setText('live-total','—');setText('live-total-sub','keine Zähler');}if(b&&lastBytes&&now>lastTime){var sec=(now-lastTime)/1000;setText('live-speed',speed((b.rx-lastBytes.rx)/sec));setText('live-speed-sub','Upload ↑ '+speed((b.tx-lastBytes.tx)/sec));}else{setText('live-speed',b?'wird berechnet':'—');setText('live-speed-sub',b?'Messung nach 5 Sekunden':'keine Zähler');}if(b){lastBytes=b;lastTime=now;}\
var warnings=[];if(!connected)warnings.push('Uplink ist getrennt.');if(rssi!==null&&rssi<-78)warnings.push('Das Uplink-Signal ist sehr schwach ('+rssi+' dBm).');if(doc.body.textContent.indexOf('No Password Protection')>=0)warnings.push('Die Weboberfläche ist nicht mit einem Passwort geschützt.');var box=document.getElementById('dashboard-warning');if(box){box.textContent=warnings.join(' ');box.className=warnings.length?'warning-box show':'warning-box';}\
var live=document.getElementById('live-dot');if(live)live.className=connected?'live-dot':'live-dot offline';setText('last-update','Aktualisiert '+new Date().toLocaleTimeString());\
}\
function request(url,done,fail){var x=new XMLHttpRequest(),finished=false;x.open('GET',url+(url.indexOf('?')>=0?'&':'?')+'_live='+Date.now(),true);x.timeout=3500;x.onreadystatechange=function(){if(x.readyState===4&&!finished){finished=true;if(x.status>=200&&x.status<300)done(x.responseText,x.responseURL||url);else if(fail)fail();}};x.ontimeout=function(){if(!finished){finished=true;if(fail)fail();}};x.onerror=function(){if(!finished){finished=true;if(fail)fail();}};x.send();}\
function statusPoll(manual){if(document.hidden&&!manual){scheduleStatus();return;}request('/',function(html){var d=new DOMParser().parseFromString(html,'text/html');updateFromDoc(d);scheduleStatus();},function(){var dot=document.getElementById('live-dot');if(dot)dot.className='live-dot offline';setText('last-update','Aktualisierung fehlgeschlagen');scheduleStatus();});}\
function scheduleStatus(){clearTimeout(statusTimer);statusTimer=setTimeout(function(){statusPoll(false);},5000);}\
function clientTableFromDoc(doc){var hs=doc.querySelectorAll('h2'),table=null;for(var i=0;i<hs.length;i++){if(/Connected Clients/i.test(text(hs[i]))){table=hs[i].nextElementSibling;break;}}return table&&table.tagName==='TABLE'?table:null;}\
function renderClients(doc,url){var table=clientTableFromDoc(doc),body=document.getElementById('client-body'),note=document.getElementById('clients-note');if(!body)return;body.innerHTML='';if(!table){body.innerHTML=\"<tr><td colspan='5' class='client-empty'>Anmelden, um verbundene Geräte anzuzeigen.</td></tr>\";if(note)note.textContent='geschützt';return;}var heads=table.querySelectorAll('th'),hasTraffic=false;for(var h=0;h<heads.length;h++)if(/Traffic/i.test(text(heads[h])))hasTraffic=true;var rows=table.querySelectorAll('tbody tr'),count=0;for(var i=0;i<rows.length;i++){var c=rows[i].querySelectorAll('td');if(c.length<3||/No connected/i.test(text(rows[i])))continue;var mac=text(c[0]),ip=text(c[1]),name=text(c[2])||'Unbekannt',traffic=hasTraffic&&c.length>3?text(c[3]):'Statistik aus';var tr=document.createElement('tr');tr.innerHTML=\"<td><div class='device-name'></div></td><td></td><td></td><td></td><td><a class='manage-link' href='/mappings'>Verwalten</a></td>\";tr.querySelector('.device-name').textContent=name;tr.children[1].textContent=ip;tr.children[2].textContent=mac;tr.children[3].textContent=traffic;body.appendChild(tr);count++;}if(!count)body.innerHTML=\"<tr><td colspan='5' class='client-empty'>Aktuell ist kein Gerät verbunden.</td></tr>\";if(note)note.textContent=count+' online';}\
function clientsPoll(manual){if(document.hidden&&!manual){scheduleClients();return;}request('/mappings',function(html,url){var d=new DOMParser().parseFromString(html,'text/html');renderClients(d,url);scheduleClients();},function(){setText('clients-note','nicht erreichbar');scheduleClients();});}\
function scheduleClients(){clearTimeout(clientsTimer);clientsTimer=setTimeout(function(){clientsPoll(false);},20000);}\
window.liveRefresh=function(){clearTimeout(statusTimer);clearTimeout(clientsTimer);statusPoll(true);clientsPoll(true);};\
document.addEventListener('DOMContentLoaded',function(){updateFromDoc(document);clientsPoll(true);scheduleStatus();});\
document.addEventListener('visibilitychange',function(){if(!document.hidden)window.liveRefresh();});\
})();\
</script>\
</head>\
<body>\
<div id='container'>\
<div class='header'>\
<div class='brand'>\
<a href='/'><img src='/favicon.png' alt='Home'></a>\
<div><h1>" INDEX_TITLE "</h1><span class='edition'>Andi Edition · Live Dashboard</span></div>\
</div>"
/* Logout button streamed here */

#define INDEX_CHUNK_STATUS_OPEN "\
</div>\
<div class='livebar'>\
<div class='live-state'><span id='live-dot' class='live-dot'></span><span id='last-update'>Live-Daten werden geladen</span></div>\
<button class='refresh-button' type='button' onclick='liveRefresh()'>Jetzt aktualisieren</button>\
</div>\
<div id='dashboard-warning' class='warning-box'></div>\
<div class='cards'>\
<div class='card'><div class='card-label'>Uplink</div><div id='live-uplink' class='card-value'>—</div><div id='live-uplink-sub' class='card-sub'>Verbindungsstatus</div></div>\
<div class='card'><div class='card-label'>Signal</div><div id='live-signal' class='card-value'>—</div><div id='live-signal-sub' class='card-sub'>Uplink-Qualität</div><div id='signal-bars' class='signal'><i></i><i></i><i></i><i></i></div></div>\
<div class='card'><div class='card-label'>Clients</div><div id='live-clients' class='card-value'>—</div><div id='live-clients-sub' class='card-sub'>Geräte verbunden</div></div>\
<div class='card'><div class='card-label'>Download aktuell</div><div id='live-speed' class='card-value'>—</div><div id='live-speed-sub' class='card-sub'>Datenrate wird berechnet</div></div>\
<div class='card'><div class='card-label'>Download gesamt</div><div id='live-total' class='card-value'>—</div><div id='live-total-sub' class='card-sub'>Upload gesamt</div></div>\
<div class='card'><div class='card-label'>Laufzeit</div><div id='live-uptime' class='card-value'>—</div><div id='live-uptime-sub' class='card-sub'>seit dem letzten Start</div></div>\
</div>\
<details class='details-card'>\
<summary>Technische Details</summary>\
<div class='status-table'>\
<table>"
/* Status rows streamed here */

#define INDEX_CHUNK_STATUS_CLOSE "\
</table>\
</div>\
</details>\
<div class='clients-card'>\
<div class='clients-head'><div class='clients-title'>Verbundene Geräte</div><div id='clients-note' class='clients-note'>wird geladen</div></div>\
<div class='client-wrap'>\
<table class='client-table'>\
<thead><tr><th>Gerät</th><th>IP</th><th>MAC</th><th>Daten</th><th></th></tr></thead>\
<tbody id='client-body'><tr><td colspan='5' class='client-empty'>Geräteliste wird geladen …</td></tr></tbody>\
</table>\
</div>\
</div>"

#if !CONFIG_ETH_UPLINK
#define INDEX_CHUNK_BUTTONS "\
<div class='button-container'>\
<a href='/setup' class='nav-button'>Schnelleinrichtung</a>\
<a href='/scan' class='nav-button'>WLAN-Scan</a>\
<a href='/config' class='nav-button'>Konfiguration</a>\
<a href='/mappings' class='nav-button'>Clients &amp; Freigaben</a>\
<a href='/firewall' class='nav-button'>Firewall</a>\
<a href='/vpn' class='nav-button'>VPN</a>\
</div>"
#else
#define INDEX_CHUNK_BUTTONS "\
<div class='button-container'>\
<a href='/config' class='nav-button'>Konfiguration</a>\
<a href='/mappings' class='nav-button'>Clients &amp; Freigaben</a>\
<a href='/firewall' class='nav-button'>Firewall</a>\
<a href='/vpn' class='nav-button'>VPN</a>\
</div>"
#endif
/* Auth UI streamed here */

#define INDEX_CHUNK_TAIL "\
<div style='margin-top:1.3rem;padding-top:.85rem;border-top:1px solid rgba(255,255,255,.08);text-align:center;'>\
<span style='color:#687386;font-size:.7rem;font-family:monospace;'>v%s | Build: %s %s | ESP-IDF: "\
IDF_VER\
" | <a href='https://github.com/martin-ger/esp32_nat_router' target='_blank' rel='noopener' style='color:#00d9ff;text-decoration:none;'>Source</a></span>\
</div>\
</div>\
</body>\
</html>"