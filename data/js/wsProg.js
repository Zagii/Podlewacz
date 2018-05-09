//var wsSerw="192.168.1.144";

document.addEventListener("DOMContentLoaded", function(event) {
    debug=document.getElementById('deb');
    console.log("document On load");
    var fileName = location.pathname.split("/").slice(-1);
    //alert (fileName);
    for(i=1;i<7;i++)
    {
        addProg(i,i*10000,i*100,i,i,i%2)
        console.log("Dodaje: "+i);
    }
    

});



function addProg(i,dd,ile_s,coIle_d,sek,akt) {
    //console.log("add: "+d);
    dt=new Date(dd);
    var w = document.createElement("div"); 
    w.className="w3-row w3-center w3-padding-16 w3-section";
    if(akt)w.classList.add("w3-light-grey");
    else w.classList.add("w3-dark-grey");
    w.id="ProgW"+i;
	var k= document.createElement("div");
    k.className="w3-third w3-section";
    c1a=document.createElement("div");
    c1a.className="w3-half w3-section";
    c1a.innerHTML="<i>Utworzono</i><br><h4>"+ dt.getUTCDate().toString().padStart(2,"0")+"-"+(dt.getUTCMonth()+1).toString().padStart(2,"0")+"-"+dt.getUTCFullYear()+"</h4>";
    k.appendChild(c1a);
    c1b=document.createElement("div");
    c1b.className="w3-half w3-section";
    c1b.innerHTML="<i>Start</i><br><h4>"+ dt.getUTCHours().toString().padStart(2,"0")+":"+dt.getUTCMinutes().toString().padStart(2,"0")+":"+dt.getUTCSeconds().toString().padStart(2,"0")+"</h4>";

    k.appendChild(c1b);
    w.appendChild(k);
	
    k= document.createElement("div");
    k.className="w3-third w3-section";
    c2a=document.createElement("div");
    c2a.className="w3-half w3-section";
    c2a.innerHTML="<i>Długość[s]</i><br><h4>"+ ile_s+"</h4>";
    k.appendChild(c2a);
    c2b=document.createElement("div");
    c2b.className="w3-half w3-section";
    c2b.innerHTML="<i>Cykliczność [x dni]</i><br><h4>"+ coIle_d+"</h4>";
    k.appendChild(c2b);
	w.appendChild(k);
	
    k= document.createElement("div");
    k.className="w3-third w3-section";
   	c3a=document.createElement("div");
    c3a.className="w3-half w3-section";
    c3a.innerHTML="<i>Sekcja</i><br><h4>"+ sek+"</h4>";
    k.appendChild(c3a);
    c3b=document.createElement("div");
    c3b.className="w3-half w3-section w3-center";
    c3b.innerHTML="<div class=\"w3-full\"><i>Opcje</i></div>";
    c3b1=document.createElement("div");
    c3b1.className="w3-half w3-section";
   // c3b1.innerHTML="<input type=\"checkbox\" onclick=\"if(this.checked){}\">";
  
   if(akt)
   {
    c3b1.innerHTML="<i class=\"fas fa-toggle-on w3-xlarge\" id=\"Prog"+i+"\" data-stan='1'></i><br><i id=\"ProgTXT"+i+"\">Aktywny</i>";//<br>aktywny"; 
   }else
   {
    c3b1.innerHTML="<i class=\"fas fa-toggle-off w3-xlarge\" id=\"Prog"+i+"\" data-stan='0'></i><br><i id=\"ProgTXT"+i+"\">Nieaktywny</i>";//<br>nieaktywny";  
    
   }
   c3b1.addEventListener('click',function(){
       var el=document.getElementById("Prog"+i);
      // alert("Prog"+i+" stan="+el.getAttribute('data-stan'));
      if(el.getAttribute('data-stan')==1)
      {
        el.className="fas fa-toggle-off w3-xlarge";
        document.getElementById("ProgTXT"+i).innerHTML="Nieaktywny";
        el.setAttribute('data-stan',0);
        document.getElementById("ProgW"+i).classList.remove('w3-light-grey');
        document.getElementById("ProgW"+i).classList.add('w3-dark-grey');
      }else
      {
        el.className="fas fa-toggle-on w3-xlarge";
        document.getElementById("ProgTXT"+i).innerHTML="Aktywny";
        el.setAttribute('data-stan',1);
        document.getElementById("ProgW"+i).classList.add('w3-light-grey');
        document.getElementById("ProgW"+i).classList.remove('w3-dark-grey');
      }
   });
    c3b.appendChild(c3b1);
    c3b2=document.createElement("div");
    c3b2.className="w3-half w3-section";
    c3b2.innerHTML="<button style=\"color:red\"><i class=\"fa fa-trash-alt w3-xlarge\"></i></button><br><i>Usuń</i>";
    c3b.appendChild(c3b2);    
    k.appendChild(c3b);
    w.appendChild(k);
	
    /*d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                " <small id=\"bs"+i+"\">OFF</small>"+           
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     */
    var foo = document.getElementById("prog");
    foo.appendChild(w);
  }
 
