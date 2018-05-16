//let wsSerw="192.168.1.144";
let progN=0;
document.addEventListener("DOMContentLoaded", function(event) {
    debug=document.getElementById('deb');
    console.log("document On load");
    let fileName = location.pathname.split("/").slice(-1);
    //alert (fileName);
    delSekcje();
    for(i=1;i<7;i++)
    {
        progN=addProg(i,i*10000,i*100,i,i,i%2)
        console.log("Dodaje: "+i);
        addSekcja(i,"Sekcja "+i);
    }
    
    document.getElementById('godz').addEventListener('change', function() {
        const className = 'error-field';
        const reg = new RegExp('^[a-zA-Z]{3,}$', 'g');
    
        if (!reg.test(this.value)) {
            this.classList.add(className); //dodaję klase do pola
        } else {
            this.classList.remove(className); //usuwam klasę
        }
    });
});

function addProgBtn()
{
  let d1=new Date(0);
  let f=document.getElementById("addProgForm").elements;
  
  if(!(f["godz"].validity.valid && f["dlug"].validity.valid))  return;
 

  console.log(f["dzien"].value);
 
  d1.setUTCDate(parseInt(f["dzien"].value)+4);
  console.log(d1.toDateString());
  let h=f["godz"].value.split(":");
  console.log(h);
  d1.setUTCHours(h[0],h[1],h[2])

  let dl=f["dlug"].value;
  let co=f["co_ile"].value;
  let s=f["sekcja"].value;
  console.log("przed add: "+progN);
    
  progN=addProg(progN+1,d1,dl,co,s,false);
  console.log(" po add: "+progN);
    
}

function addProg(i,dd,ile_s,coIle_d,sek,akt) {
    console.log("add: "+i);
    
    dt=new Date(dd);
    let w = document.createElement("div"); 
    w.className="w3-row w3-center w3-padding-16 w3-section";
    if(akt)w.classList.add("w3-light-grey");
    else w.classList.add("w3-dark-grey");
    w.id="ProgW"+i;
	let k= document.createElement("div");
    k.className="w3-third w3-section";
    c1a=document.createElement("div");
    c1a.className="w3-half w3-section";
    let dzienTyg=["Niedziela","Poniedziałek","Wtorek","Środa","Czwarek","Piątek","Sobota"];
    c1a.innerHTML="<i>Dzień tygodnia</i><br><h4>"+dzienTyg[dt.getUTCDay()] /*dt.getUTCDayte().toString().padStart(2,"0")+"-"+(dt.getUTCMonth()+1).toString().padStart(2,"0")+"-"+dt.getUTCFullYear()*/+"</h4>";
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
       let el=document.getElementById("Prog"+i);
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
    c3b2.innerHTML="<button type='button' style=\"color:red\" onclick=\"delProg("+i+")\"><i class=\"fa fa-trash-alt w3-xlarge\"></i></button><br><i>Usuń</i>";
    c3b.appendChild(c3b2);    
    k.appendChild(c3b);
    w.appendChild(k);
	
    /*d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                " <small id=\"bs"+i+"\">OFF</small>"+           
                " <p class=\"w3-wide\" >Sekcja "+i+"\</p>"+
                " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     */
    let foo = document.getElementById("prog");
    foo.appendChild(w);
   return i;
  }
  function delProg(i){
    if(confirm("Czy chcesz usunąć program? "+i))
    {
        alert("usuwam");
    }
}
function delSekcje()
{
    let f=document.getElementById("addProgForm").elements["sekcja"];
   while(f.length>0)
    {
       f.remove(f.length-1);
    }
}
function addSekcja(v,n)
{
    let o = document.createElement("option");
    o.text = n;
    o.value=v;
    document.getElementById("addProgForm").elements["sekcja"].add(o);
    addSekList(v,n);
}
function addSekList(v,n)
{
    let f=document.getElementById("sekList");
    let w = document.createElement("div"); 
    w.className="w3-row w3-center w3-grey w3-padding-8 w3-section";
    //if(true)
    //w.classList.add("w3-light-grey");
    //else w.classList.add("w3-dark-grey");
    w.id="SekLbl"+v;
	//let k= document.createElement("div");
    //k.className="w3-third w3-section";
    c1a=document.createElement("div");
    c1a.className="w3-col m2 w3-section";
    c1a.innerHTML="<h4>"+v+"</h4>";
    //k.appendChild(c1a);
    c1b=document.createElement("div");
    c1b.className="w3-col m5 w3-section";
    c1b.innerHTML="<input class=\"w3-input\" style=\"width:90%\" type=\"text\" "+
            " id=\"sekLi"+i+"\" name=\"sekLi"+i+"\" value=\"\" placeholder=\"Sekcja "+v+"\" required/>";
    //k.appendChild(c1b);
    c1c=document.createElement("div");
    c1c.className="w3-col m1 w3-section ";
    c1c.innerHTML=" <button type='button' class='btn button5' style='width:80%;' onclick=\"KonfForm('SekLbl"+i+"')\"> "+
            "<i style='color:green' class='fa fa-edit w3-xlarge' aria-hidden='true' ></i></button>";
    //k.appendChild(c1c);
    w.appendChild(c1a);
    w.appendChild(c1b);
    w.appendChild(c1c);
    //w.appendChild(k);
    f.appendChild(w);
}

function KonfForm(id)
{

    let x = document.getElementById(id).elements;//.serializeArray();
    let v =true;
    for(let i=0;i<x.length;i++)
    {
    
        if(!x[i].validity.valid)  v=false;
    }
    if(!v)
    {
        console.log("blad walidacji "+id);
        //return;
    }
    
    const data = formToJSON(x);


    
 //   console.log("json="+JSON.stringify(data));
    let str={"topic":id,"msg":data};
    console.log(JSON.stringify(str));
   
}

//// IA JSON 
// setNTP  {topic:setNTP; msg:{host:adr; offset:h}} ustawia parametry zegara NTP
// setTime {topic:setTime; msg:{dzien:x;czas:str}} ustawia parametry zegara
// setWifi {topic:setWifi; msg:{typ:a/d/p; ssid:s;pass:p}} add/del/publish ssid passwd
// setMQTT {topic:setMqtt; msg:{host:adr; p:port; l:login; h:passwd}} ustawienia mqtt
// setSek  {topic:setSek;  msg:{id:i; n:nazwa}}
///


/**
 * Checks that an element has a non-empty `name` and `value` property.
 * @param  {Element} element  the element to check
 * @return {Bool}             true if the element is an input, false if not
 */
const isValidElement = element => {
    return element.name && element.value;
  };
  
  /**
   * Checks if an element’s value can be saved (e.g. not an unselected checkbox).
   * @param  {Element} element  the element to check
   * @return {Boolean}          true if the value should be added, false if not
   */
  const isValidValue = element => {
    return (!['checkbox', 'radio'].includes(element.type) || element.checked);
  };
  
  /**
   * Checks if an input is a checkbox, because checkboxes allow multiple values.
   * @param  {Element} element  the element to check
   * @return {Boolean}          true if the element is a checkbox, false if not
   */
  const isCheckbox = element => element.type === 'checkbox';
  const isBtn = element => element.type === 'button';
  
  /**
   * Checks if an input is a `select` with the `multiple` attribute.
   * @param  {Element} element  the element to check
   * @return {Boolean}          true if the element is a multiselect, false if not
   */
  const isMultiSelect = element => element.options && element.multiple;
  
  /**
   * Retrieves the selected options from a multi-select as an array.
   * @param  {HTMLOptionsCollection} options  the options for the select
   * @return {Array}                          an array of selected option values
   */
  const getSelectValues = options => [].reduce.call(options, (values, option) => {
    return option.selected ? values.concat(option.value) : values;
  }, []);
  
  /**
   * A more verbose implementation of `formToJSON()` to explain how it works.
   *
   * NOTE: This function is unused, and is only here for the purpose of explaining how
   * reducing form elements works.
   *
   * @param  {HTMLFormControlsCollection} elements  the form elements
   * @return {Object}                               form data as an object literal
   */
  const formToJSON_deconstructed = elements => {
    
    // This is the function that is called on each element of the array.
    const reducerFunction = (data, element) => {
      
      // Add the current field to the object.
      data[element.name] = element.value;
      
      // For the demo only: show each step in the reducer’s progress.
      console.log(JSON.stringify(data));
  
      return data;
    };
    
    // This is used as the initial value of `data` in `reducerFunction()`.
    const reducerInitialValue = {};
    
    // To help visualize what happens, log the inital value, which we know is `{}`.
    console.log('Initial `data` value:', JSON.stringify(reducerInitialValue));
    
    // Now we reduce by `call`-ing `Array.prototype.reduce()` on `elements`.
    const formData = [].reduce.call(elements, reducerFunction, reducerInitialValue);
    
    // The result is then returned for use elsewhere.
    return formData;
  };
  
  /**
   * Retrieves input data from a form and returns it as a JSON object.
   * @param  {HTMLFormControlsCollection} elements  the form elements
   * @return {Object}                               form data as an object literal
   */
  const formToJSON = elements => [].reduce.call(elements, (data, element) => {
  
    // Make sure the element has the required properties and should be added.
    if (isValidElement(element) && isValidValue(element)) {
  
      /*
       * Some fields allow for more than one value, so we need to check if this
       * is one of those fields and, if so, store the values as an array.
       */
      if(isBtn(element))return;
      if (isCheckbox(element)) {
        data[element.name] = (data[element.name] || []).concat(element.value);
      } else if (isMultiSelect(element)) {
        data[element.name] = getSelectValues(element);
      } else {
        data[element.name] = element.value;
      }
    }
  
    return data;
  }, {});
  
  