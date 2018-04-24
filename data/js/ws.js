//var wsSerw="192.168.1.144";
var debug;//=document.getElementById('deb');
function deb(t)
{
    debug.innerHTML+=t+"<br>";
}
var connection = new WebSocket('ws://'+wsSerw+':81/'); 
connection.onopen = function () { connection.send('Connect ' + new Date()); };
connection.onerror = function (error) { console.log('WebSocket Error ', error); }; 
connection.onmessage = function (e) 
{
     console.log('Server: ', e.data); 
     
     deb("onMessage: "+e.data);
}; 

document.addEventListener("DOMContentLoaded", function(event) {
    // - Code to execute when all DOM content is loaded. 
    // - including fonts, images, etc.
    debug=document.getElementById('deb');
    console.log("document On load");
    for(i=1;i<7;i++)
    {
        add(i);
       // setStan(i,i%2);
        console.log("Dodaje: "+i);
    }
});

function sendRGB() { 
    var r = parseInt(document.getElementById('r').value).toString(16); 
    var g = parseInt(document.getElementById('g').value).toString(16); 
    var b = parseInt(document.getElementById('b').value).toString(16); 
    if (r.length < 2) { r = '0' + r; } 
    if (g.length < 2) { g = '0' + g; } 
    if (b.length < 2) { b = '0' + b; } 
    var rgb = '#' + r + g + b; 
    console.log('RGB: ' + rgb); 
  //  connection.send(rgb); 
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
    deb("setStan : nr="+nr+", stan="+stan);
}
function sendStan( nr)
{
    var s=document.getElementById("bs"+nr);
    var w=0;
    if(s.innerHTML=="ON")w=1;
    
    var msg=nr+",stan="+w;
    console.log("sendStan msg="+msg );
    deb("sendStan msg="+msg);
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
    //Append the element in page (in span).  
    foo.appendChild(d);
  }
 
