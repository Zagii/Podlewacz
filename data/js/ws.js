//var connection = new WebSocket('ws://location.hostname:81/', ['arduino']); 
//connection.onopen = function () { connection.send('Connect ' + new Date()); };
//connection.onerror = function (error) { console.log('WebSocket Error ', error); }; 
//connection.onmessage = function (e) { console.log('Server: ', e.data); }; 

document.addEventListener("DOMContentLoaded", function(event) {
    // - Code to execute when all DOM content is loaded. 
    // - including fonts, images, etc.

    console.log("document On load");
    for(i=0;i<6;i++)
    {
        add(i);
        console.log("Dodaje: "+i);
    }
});
/*window.onload = function() 
{
    console.log("window On load");
    for(i=0;i<6;i++)
    {
        add(i);
        console.log("Dodaje: "+i);
    }
}*/
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

function sendStan( nr, stan)
{

    var msg=nr+","+stan;
    console.log("sendStan msg="+msg +"->"+ document.getElementById(nr).innerText);
}

function add(i) {

    var d = document.createElement("div"); 
    console.log("add: "+d);
    d.className="w3-third w3-section";
    d.innerHTML= "<button type=\"submit\" class=\"button button5\" id="+i+" onclick=\"sendStan("+i+","+i*10+")>"+
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\"></i></button>";
      
    var foo = document.getElementById("sDiv");
    //Append the element in page (in span).  
    foo.appendChild(d);
  }
 
 