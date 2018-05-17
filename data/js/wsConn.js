//var wsSerw="192.168.1.144";
class global
{
    constructor()
    {
        this.dTyg=["Niedziela","Poniedziałek","Wtorek","Środa","Czwarek","Piątek","Sobota"];
        this.sLbl=[];
        this.dt=new Date();   
    }
    get dzien(i){return this.dTyg[i];}
    set lbl(i,s){this.sLbl[i]=s;}
    get lbl(i){return this.sLbl[i];}
    setCzas(mSek)
    {
        dt.setTime(mSek*1000);
    }
    getGodz(){
        return dt.getUTCHours().toString().padStart(2,"0")+":"+
               dt.getUTCMinutes().toString().padStart(2,"0")+":"+
               dt.getUTCSeconds().toString().padStart(2,"0");       
    }
    getDtStr()
    {
        return dt.getUTCDate().toString().padStart(2,"0")+"-"+
                (dt.getUTCMonth()+1).toString().padStart(2,"0")+"-"+
                dt.getUTCFullYear();
    }
    getDTyg() {return dTyg[dt.getUTCDay()];}
}

class wsConn {
    constructor(fCon,fDc,fMsg) {
        this.ws=null;
        this.fCon=fCon;
        this.fDc=fDc;
        this.fMsg=fMsg;
        setInterval(checkWS, 5000);
    }
    checkWS()
    {
        console.log("checkWS");
        if(ws)
        {
            if(ws.readyState == WebSocket.CLOSED)
            {          
                startWS();
            }else
            {
                if(ws.readyState==WebSocket.OPEN)
                {
                } else
                {
                }
            }
        }
        return this.ws.readyState;
    }
    startWS()
    {
        console.log("startWS");
        this.ws = new WebSocket('ws://'+wsSerw+':81/'); 
        this.ws.onopen = function () { 
            let j={ "topic":"status", "msg":"Connected" };
            send(JSON.stringify(j));
            this.fCon();
        };
        this.ws.onerror = function (error) { 
            console.log('WebSocket Error ', error); 
            delete this.ws; 
            this.fDc();
	    }; 
        this.ws.onmessage = function (e) 
        {
        //  }
        // nr=parseInt("00100000", 2).toString(10);
        ///deb("a="+nr);
        //         let dt="{\"GEO\":\"123\",\"TEMP\":124.4,\"TRYB\":0,\"CZAS\":123,\"CISN\":333,\"DESZCZ\":0,\"SEKCJE\":"+nr+"}";
        // console.log('ws recive: ', e.data); 
            //deb("onMessage: "+e.data);
            let j=JSON.parse(e.data); 
            this.fMsg(j);
        }; 
    }
    send(m){if(this.ws) ws.send(m);}
}


