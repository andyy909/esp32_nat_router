/* Simple access-mode page */
#pragma once

#define ACCESS_PAGE_HEAD "<!doctype html>\
<html lang='en'><head><meta charset='UTF-8'>\
<meta name='viewport' content='width=device-width,initial-scale=1'>\
<title>Access Mode</title><link rel='icon' href='/favicon.png'>\
<style>\
*{box-sizing:border-box;margin:0;padding:0}body{font-family:Arial,sans-serif;background:#111827;color:#e5e7eb;min-height:100vh;padding:1rem;line-height:1.45}#container{max-width:720px;margin:auto;padding:1.25rem;background:#1c2030;border:1px solid #304052;border-radius:8px}.header{display:flex;align-items:center;gap:.8rem;margin-bottom:1.2rem}.header img{width:52px;height:52px}.header h1{font-size:1.35rem;color:#35d7ee}.back{margin-left:auto;color:#a5eef7;text-decoration:none;border:1px solid #3b5968;border-radius:6px;padding:.45rem .65rem}.presets{display:grid;grid-template-columns:repeat(2,1fr);gap:.6rem;margin-bottom:1rem}.preset{border:1px solid #426174;background:#202b3d;color:#dffaff;border-radius:6px;padding:.7rem;font-weight:600}.settings{border-block:1px solid #344153}.setting{display:flex;align-items:center;gap:.75rem;padding:1rem 0;border-bottom:1px solid #2b3545}.setting:last-child{border:0}.setting input{width:21px;height:21px;accent-color:#22c5d9}.setting label{font-weight:650}.setting small{display:block;color:#9ca3af;font-weight:400;margin-top:.18rem}.summary{margin:1rem 0;padding:.8rem;border-left:3px solid #35d7ee;background:#172334;color:#c9f7fc}.save{width:100%;border:0;border-radius:6px;padding:.85rem;background:#16a6b8;color:#fff;font-size:1rem;font-weight:700}.saved{margin-bottom:1rem;padding:.7rem;background:#153a2b;color:#9ce6bd;border:1px solid #286646;border-radius:6px}@media(max-width:520px){body{padding:.45rem}#container{padding:.85rem}.presets{grid-template-columns:1fr}}\
</style><script>\
function setMode(i,c,p){document.getElementById('acc_inet').checked=i;document.getElementById('acc_clients').checked=c;document.getElementById('acc_private').checked=p;summary()}\
function summary(){var i=document.getElementById('acc_inet').checked,c=document.getElementById('acc_clients').checked,p=document.getElementById('acc_private').checked,a=[];a.push(i?'Internet allowed':'Internet blocked');a.push(c?'Client links allowed':'Clients isolated');a.push(p?'Private uplink reachable':'Private uplink blocked');document.getElementById('summary').textContent=a.join(' · ')}\
document.addEventListener('DOMContentLoaded',summary);\
</script></head><body><div id='container'><div class='header'><a href='/'><img src='/favicon.png' alt='Home'></a><h1>Access Mode</h1><a class='back' href='/'>Back</a></div>"

#define ACCESS_PAGE_FORM "\
<div class='presets'>\
<button class='preset' type='button' onclick='setMode(true,false,false)'>Internet Only</button>\
<button class='preset' type='button' onclick='setMode(false,true,false)'>Client LAN Only</button>\
<button class='preset' type='button' onclick='setMode(true,true,false)'>Clients + Internet</button>\
<button class='preset' type='button' onclick='setMode(true,true,true)'>Allow All</button>\
</div><form action='/access' method='POST'><div class='settings'>\
<div class='setting'><input id='acc_inet' name='acc_inet' type='checkbox' value='1' %s onchange='summary()'><label for='acc_inet'>Share Internet<small>Public Internet access</small></label></div>\
<div class='setting'><input id='acc_clients' name='acc_clients' type='checkbox' value='1' %s onchange='summary()'><label for='acc_clients'>Connect Clients<small>Traffic between ESP32 clients</small></label></div>\
<div class='setting'><input id='acc_private' name='acc_private' type='checkbox' value='1' %s onchange='summary()'><label for='acc_private'>Private Uplink<small>Access private uplink networks</small></label></div>\
</div><div id='summary' class='summary'></div><button class='save' type='submit'>Save</button></form>"

#define ACCESS_PAGE_TAIL "</div></body></html>"
