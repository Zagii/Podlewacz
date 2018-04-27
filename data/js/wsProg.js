//var wsSerw="192.168.1.144";
var debug;
function deb(t)
{
    debug.innerHTML+=t+"<br>";
}
var ws=null;
function checkWS(){
    console.log("checkWS");
    if(!ws || ws.readyState == WebSocket.CLOSED) startWS();
  }
  
function startWS()
{
    console.log("startWS");
    ws = new WebSocket('ws://'+wsSerw+':81/'); 
    ws.onopen = function () { ws.send('Connect ' + new Date()); };
    ws.onerror = function (error) { console.log('WebSocket Error ', error); delete ws; }; 
    ws.onmessage = function (e) 
    //     function a(nr)
    {
    //  }
    // nr=parseInt("00100000", 2).toString(10);
    ///deb("a="+nr);
    //         var dt="{\"GEO\":\"123\",\"TEMP\":124.4,\"TRYB\":0,\"CZAS\":123,\"CISN\":333,\"DESZCZ\":0,\"SEKCJE\":"+nr+"}";
        console.log('Server: ', e.data); 
        deb("onMessage: "+e.data);
        var json=JSON.parse(e.data); 
    //  var json=JSON.parse(dt); 
        if(json.hasOwnProperty("CZAS"))
        {
                var d=new Date(parseInt(json["CZAS"])*1000);
                document.getElementById('godz').innerHTML=d.getHours().toString().padStart(2,"0")+":"+d.getMinutes().toString().padStart(2,"0")+":"+d.getSeconds().toString().padStart(2,"0");
                document.getElementById('dX').innerHTML=d.getDay().toString().padStart(2,"0")+"-"+d.getMonth().toString().padStart(2,"0")+"-"+d.getFullYear();
        }
        if(json.hasOwnProperty("GEO"))
        { 
                document.getElementById('geo').innerHTML=json["GEO"];
        }
        if(json.hasOwnProperty("TEMP"))
        { 
                document.getElementById('temp').innerHTML=json["TEMP"];
        }
        if(json.hasOwnProperty("CISN"))
        { 
                document.getElementById('cisn').innerHTML=json["CISN"];
        }
        if(json.hasOwnProperty("DESZCZ"))
        { 
                document.getElementById('deszcz').innerHTML=json["DESZCZ"];
        }
        if(json.hasOwnProperty("TRYB"))
        { 
                document.getElementById('tryb').innerHTML=json["TRYB"];
        }
        if(json.hasOwnProperty("SEKCJE"))
        { 
                setStany(json["SEKCJE"]);
        }
    }; 
};
document.addEventListener("DOMContentLoaded", function(event) {
    debug=document.getElementById('deb');
    console.log("document On load");
    for(i=1;i<7;i++)
    {
        addProg(i,i*10000,i*100,i,i)
        console.log("Dodaje: "+i);
    }
    startWS();

    setInterval(checkWS, 5000);
});

function setStany(sekcje)
{
   // deb("setStan sekcje="+sekcje);
    for(i=1;i<7;i++)
    {
     //   deb("i="+i+" w="+sekcje&(1<<i));
        setStan(i,sekcje&(1<<i));
    }
}
function setStan(nr,stan)
{
    
    var s=document.getElementById("bs"+nr);
    var i=document.getElementById("bi"+nr);
    if(stan==0)
    {
        s.innerHTML="OFF";
        i.style.color="darkred";
    }else
    {
        s.innerHTML="ON";
        i.style.color="lime";
    }
  //  deb("setStan : nr="+nr+", stan="+stan);
}
function sendStan( nr)
{
    
    var s=document.getElementById("bs"+nr);
    var w=0;
    if(s.innerHTML=="OFF")w=1;
  //  a(w<<nr);
    var jsonOb={ "typ":"SEKCJA", "id":nr, "wart":w };
    var msg=JSON.stringify(jsonOb);
    console.log("sendStan msg="+msg );
  //  deb("sendStan msg="+msg);
    if(ws) ws.send(msg);   
}

function addProg(i,dd,ile_s,coIle_d,sek) {
    //console.log("add: "+d);
    dt=new Date(dd);
    var w = document.createElement("div"); 
    w.className="w3-third w3-section";
    c1a=document.createElement("div");
    c1a.className="w3-half w3-section";
    c1a.innerHTML= dt.getDay().toString().padStart(2,"0")+"-"+dt.getMonth().toString().padStart(2,"0")+"-"+dt.getFullYear();
    w.appendChild(c1a);
    c1b=document.createElement("div");
    c1b.className="w3-half w3-section";
    c1b.innerHTML= dt.getHours().toString().padStart(2,"0")+":"+dt.getMinutes().toString().padStart(2,"0")+":"+dt.getSeconds().toString().padStart(2,"0");
    w.appendChild(c1b);
    
    c2a=document.createElement("div");
    c2a.className="w3-half w3-section";
    c2a.innerHTML= ile_s;
    w.appendChild(c2a);
    c2b=document.createElement("div");
    c2b.className="w3-half w3-section";
    c2b.innerHTML= coIle_d;
    w.appendChild(c2b);

    c3a=document.createElement("div");
    c3a.className="w3-half w3-section";
    c3a.innerHTML= sek;
    w.appendChild(c3a);
    c3b=document.createElement("div");
    c3b.className="w3-half w3-section";
    c3b.innerHTML= "button";
    w.appendChild(c3b);
    
    /*d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                " <small id=\"bs"+i+"\">OFF</small>"+           
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     */
    var foo = document.getElementById("prog");
    foo.appendChild(w);
  }
 
