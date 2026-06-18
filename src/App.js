import { useState, useEffect, useRef } from "react";
import mqtt from "mqtt";

function App(){

const [status,setStatus]=useState("Offline");
const [berat,setBerat]=useState(0);
const [tpm,setTpm]=useState(0);
const [target,setTarget]=useState(20);
const [servo,setServo]=useState(0);
const [statusInfus,setStatusInfus]=useState("NORMAL");
const [isLogin,setIsLogin]=useState(false);

const [username,setUsername]=useState("");
const [password,setPassword]=useState("");

const clientRef=useRef(null);

useEffect(()=>{

clientRef.current=mqtt.connect(
"wss://870c766cdeac4276a1d1addf8b853c55.s1.eu.hivemq.cloud:8884/mqtt",
{
username:"Infuspintar",
password:"Infuspintar1234_"
}
);

clientRef.current.on("connect",()=>{

setStatus("Online");

console.log("MQTT Connected");

clientRef.current.subscribe(
"infus/berat"
);

clientRef.current.subscribe(
"infus/tpm"
);
clientRef.current.subscribe(
"infus/servo"
);

clientRef.current.subscribe(
"infus/status"
);

});

clientRef.current.on(
"message",
(topic,message)=>{

let data=
message.toString();

if(topic==="infus/berat"){

setBerat(data);

}

if(topic==="infus/tpm"){
  setTpm(data);
}
if(topic==="infus/servo")
{
  setServo(data);
}

if(topic==="infus/status")
{
  setStatusInfus(data);
}

}
);

return ()=>{

clientRef.current.end();

}

},[]);

function login()
{
  if(
    username === "infus123" &&
    password === "infus123"
  )
  {
    setIsLogin(true);
  }
  else
  {
    alert("Username atau Password Salah");
  }
}
function kirimTarget(){

clientRef.current.publish(

"infus/target",

target.toString()

);

alert(
"TPM terkirim: "+target
);

}

if(!isLogin)
{
  return(

<div style={{
background:"linear-gradient(135deg,#1976d2,#42a5f5)",
minHeight:"100vh",
display:"flex",
justifyContent:"center",
alignItems:"center"
}}>

<div style={{
background:"white",
padding:"40px",
borderRadius:"20px",
width:"350px",
boxShadow:"0 10px 25px rgba(0,0,0,0.2)"
}}>

<h1
style={{
textAlign:"center",
color:"#1976d2",
marginBottom:"5px"
}}
>
SMART INFUS
</h1>

<p
style={{
textAlign:"center",
color:"gray",
marginBottom:"25px"
}}
>
Monitoring Infus Berbasis IoT
</p>

<input
placeholder="Username"
value={username}
onChange={(e)=>
setUsername(e.target.value)
}
style={{
width:"100%",
padding:"12px",
borderRadius:"10px",
border:"1px solid #ccc",
boxSizing:"border-box"
}}
/>

<br/><br/>

<input
type="password"
placeholder="Password"
value={password}
onChange={(e)=>
setPassword(e.target.value)
}
style={{
width:"100%",
padding:"12px",
borderRadius:"10px",
border:"1px solid #ccc",
boxSizing:"border-box"
}}
/>


<br/><br/>

<button
onClick={login}
style={{
width:"100%",
padding:"12px",
background:"#1976d2",
color:"white",
border:"none",
borderRadius:"10px",
fontSize:"16px",
fontWeight:"bold",
cursor:"pointer"
}}
>
LOGIN
</button>

</div>

</div>

  );
}
return(

<div style={{
minHeight:"100vh",
background:"#f4f7fb",
padding:"30px"
}}>

<h1
style={{
textAlign:"center",
color:"#1976d2"
}}
>
SMART INFUS MONITORING
</h1>

<div
style={{
textAlign:"center",
marginBottom:"20px"
}}
>
Status MQTT :
<span
style={{
color:"green",
fontWeight:"bold"
}}
>
 {status}
</span>
</div>

<h1>
Dashboard Monitoring Infus
</h1>

<h3>
Status:
{status}
</h3>

<hr/>

<div
style={{
background:"white",
padding:"25px",
borderRadius:"20px",
boxShadow:"0 5px 20px rgba(0,0,0,0.1)",
maxWidth:"500px",
margin:"auto"
}}
>

<h2>
🛏 KAMAR 1
</h2>

<hr/>

<h3>
💧 Berat : {berat} gram
</h3>

<h3>
📈 TPM : {tpm}
</h3>

<h3>
⚙ Servo : {servo}°
</h3>

<h3>
Status :
{
statusInfus==="NORMAL"
?
" 🟢 NORMAL"
:
statusInfus==="HAMPIR HABIS"
?
" 🟡 HAMPIR HABIS"
:
" 🔴 INFUS MACET"
}
</h3>

</div>

<hr/>

<div
style={{
background:"white",
padding:"20px",
marginTop:"20px",
borderRadius:"20px",
boxShadow:"0 5px 20px rgba(0,0,0,0.1)",
maxWidth:"500px",
marginLeft:"auto",
marginRight:"auto"
}}
>

<h3>
🎯 Target TPM
</h3>

<input
type="number"
value={target}
onChange={(e)=>
setTarget(e.target.value)
}
style={{
padding:"10px",
width:"120px",
borderRadius:"10px",
border:"1px solid #ccc"
}}
/>

<button
onClick={kirimTarget}
style={{
marginLeft:"10px",
padding:"10px 20px",
background:"#1976d2",
color:"white",
border:"none",
borderRadius:"10px",
cursor:"pointer"
}}
>
Kirim
</button>

</div>

<input
type="number"
value={target}
onChange={(e)=>
setTarget(
e.target.value
)}
/>

<button
onClick={kirimTarget}
>

Kirim

</button>

</div>

)

}

export default App;