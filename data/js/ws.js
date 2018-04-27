//var wsSerw="192.168.1.144";
var debug;
function deb(t)
{
    debug.innerHTML+=t+"<br>";
}
var connection = new WebSocket('ws://'+wsSerw+':81/'); 
connection.onopen = function () { connection.send('Connect ' + new Date()); };
connection.onerror = function (error) { console.log('WebSocket Error ', error); }; 
connection.onmessage = function (e) 
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
            document.getElementById('data').innerHTML=d.getDay().toString().padStart(2,"0")+"-"+d.getMonth().toString().padStart(2,"0")+"-"+d.getFullYear();
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

document.addEventListener("DOMContentLoaded", function(event) {
    debug=document.getElementById('deb');
    console.log("document On load");
    for(i=1;i<7;i++)
    {
        add(i);
        console.log("Dodaje: "+i);
    }
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
    connection.send(msg);   
}

function add(i) {
    var d = document.createElement("div"); 
    console.log("add: "+d);
    d.className="w3-third w3-section";
    d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                " <small id=\"bs"+i+"\">OFF</small>"+           
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     
    var foo = document.getElementById("sDiv");
    foo.appendChild(d);
  }
 
