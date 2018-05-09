//var wsSerw="192.168.1.144";

document.addEventListener("DOMContentLoaded", function(event) {
    debug=document.getElementById('deb');
    console.log("document On load");
    var fileName = location.pathname.split("/").slice(-1);
    alert (fileName);
    for(i=1;i<7;i++)
    {
        addProg(i,i*10000,i*100,i,i)
        console.log("Dodaje: "+i);
    }
    

});



function addProg(i,dd,ile_s,coIle_d,sek) {
    //console.log("add: "+d);
    dt=new Date(dd);
    var w = document.createElement("div"); 
	w.className="w3-row w3-center w3-padding-16 w3-section w3-light-grey";
	var k= document.createElement("div");
    k.className="w3-third w3-section";
    c1a=document.createElement("div");
    c1a.className="w3-half w3-section";
    c1a.innerHTML="<h3>"+ dt.getDay().toString().padStart(2,"0")+"-"+dt.getMonth().toString().padStart(2,"0")+"-"+dt.getFullYear()+"</h3>";
    k.appendChild(c1a);
    c1b=document.createElement("div");
    c1b.className="w3-half w3-section";
    c1b.innerHTML="<h3>"+ dt.getHours().toString().padStart(2,"0")+":"+dt.getMinutes().toString().padStart(2,"0")+":"+dt.getSeconds().toString().padStart(2,"0")+"</h3>";
    k.appendChild(c1b);
    w.appendChild(k);
	
    k= document.createElement("div");
    k.className="w3-third w3-section";
    c2a=document.createElement("div");
    c2a.className="w3-half w3-section";
    c2a.innerHTML="<h3>"+ ile_s+"</h3>";
    k.appendChild(c2a);
    c2b=document.createElement("div");
    c2b.className="w3-half w3-section";
    c2b.innerHTML="<h3>"+ coIle_d+"</h3>";
    k.appendChild(c2b);
	w.appendChild(k);
	
    k= document.createElement("div");
    k.className="w3-third w3-section";
   	c3a=document.createElement("div");
    c3a.className="w3-half w3-section";
    c3a.innerHTML="<h3>"+ sek+"</h3>";
    k.appendChild(c3a);
    c3b=document.createElement("div");
    c3b.className="w3-half w3-section";
    c3b.innerHTML="<h3>"+ "button"+"</h3>";
    k.appendChild(c3b);
    w.appendChild(k);
	
    /*d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                " <small id=\"bs"+i+"\">OFF</small>"+           
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     */
    var foo = document.getElementById("prog");
    foo.appendChild(w);
  }
 
