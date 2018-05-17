//var wsSerw="192.168.1.144";
class global
{
    constructor()
    {
        this.dTyg=["Niedziela","Poniedziałek","Wtorek","Środa","Czwarek","Piątek","Sobota"];
        this.sLbl=[];
        this.dt=new Date();   
    }
    getDzien(i){return this.dTyg[i];}
    setLbl(i,s){this.sLbl[i]=s;}
    geLbl(i){return this.sLbl[i];}
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

class wsConn
{
    constructor(fCon,fDc,fMsg) {
        this.ws=null;
        this.fCon=fCon.bind(this);
        this.fDc=fDc.bind(this);
        this.fMsg=fMsg.bind(this);
        
    }
    begin(d)
    {
        setInterval(this.checkWS.bind(this), d*1000);
    }
    checkWS()
    {
      //  console.log("checkWS");
        if(this.ws)
        {
            if(this.ws.readyState == WebSocket.CLOSED)
            {          
                this.startWS();
            }else
            {
                if(this.ws.readyState==WebSocket.OPEN)
                {
                } else
                {
                }
            }
        
        }else
        {
            this.startWS();
        }
        return this.ws.readyState;
       
    }
    startWS()
    {
        console.log("startWS");
      //  this.ws = new WebSocket('ws://'+wsSerw+':81/'); 
        this.ws =new WebSocket("wss://echo.websocket.org/");
        let me=this;
        this.ws.onopen = function () 
        { 
            let j={ "topic":"status", "msg":"Connected" };
            this.send(JSON.stringify(j));
            me.fCon();
        };
        this.ws.onerror = function (error) 
        { 
            console.log('WebSocket Error ', error); 
            delete this.ws; 
            me.fDc();
	    }; 
        this.ws.onmessage = function (e) 
        {
            let j=JSON.parse(e.data); 
            me.fMsg(j);
        };
        return this.ws.readyState; 
    }
    send(m)
    {
        if(this.ws) this.ws.send(m);
    }
    sendtest()
    {
        let t="{\"t\":\""+new Date().toISOString()+"\"}";
        this.send(t);
    }
}


