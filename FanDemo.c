#include<reg52.h>
#include<intrins.h>
#define uint unsigned int
#define uchar unsigned char

sbit key_1=P2^3;  
sbit key_2=P2^2;
sbit key_3=P2^1;
sbit key_4=P2^0;

sbit DS=P2^7; 
sbit mos=P3^3;
sbit LM358=P2^6;

uint num_0,round_t,temp,time_num;  //temp温度传感器部分变量	a蓝牙模块变量
float t;  //温度
uint pp,il;
uint STANDBY;

int code num[]={
 	0xa0,0xbb,0x62,0x2a,0x39,0x2c,0x24,0xba,0x20,0x28,0xff  //数字0-9	
};
int code num_point[]={
  	0x80,0x9b,0x42,0x0a,0x19,0x0c,0x04,0x9a,0x00,0x08,0x7f  //数字0.-9.和-
};
int code WELA[]={
  	0xfe,0xfd,0xfb,0xf7
};

/*********蓝牙板块********/
//未完成
/********蓝牙分割线*******/

/***********温度传感器*********/
void delay(uint count)      //delay
{
  uint i;
  while(count)
  {
    i=200;
    while(i>0)
    i--;
    count--;
  }
}
void dsreset(void)       //send reset and initialization command
{
  uint i;
  DS=0;
  i=103;
  while(i>0)i--;
  DS=1;
  i=4;
  while(i>0)i--;
}

bit tmpreadbit(void)       //read a bit
{
   uint i;
   bit dat;
   DS=0;i++;          //i++ for delay
   DS=1;i++;i++;
   dat=DS;
   i=8;while(i>0)i--;
   return (dat);
}

uchar tmpread(void)   //read a byte date
{
  uchar i,j,dat;
  dat=0;
  for(i=1;i<=8;i++)
  {
    j=tmpreadbit();
    dat=(j<<7)|(dat>>1);   //读出的数据最低位在最前面，这样刚好一个字节在DAT里
  }
  return(dat);
}

void tmpwritebyte(uchar dat)   //write a byte to ds18b20
{
  uint i;
  uchar j;
  bit testb;
  for(j=1;j<=8;j++)
  {
    testb=dat&0x01;
    dat=dat>>1;
    if(testb)     //write 1
    {
      DS=0;
      i++;i++;
      DS=1;
      i=8;while(i>0)i--;
    }
    else
    {
      DS=0;       //write 0
      i=8;while(i>0)i--;
      DS=1;
      i++;i++;
    }

  }
}

void tmpchange(void)  //DS18B20 begin change
{
  dsreset();
  delay(1);
  tmpwritebyte(0xcc);  // address all drivers on bus
  tmpwritebyte(0x44);  //  initiates a single temperature conversion
}

float tmp()               //get the temperature
{
  float tt;
  uchar a,b;
  dsreset();
  delay(1);
  tmpwritebyte(0xcc);
  tmpwritebyte(0xbe);
  a=tmpread();
  b=tmpread();
  temp=b;
  temp<<=8;             //two byte  compose a int variable
  temp=temp|a;
  tt=temp*0.0625;
//  temp=tt*10+0.5;
  return tt;
}
/*********温度传感器分隔线*********/


/**
* 1ms/单位 延迟
*uint t --->>单位大小
*/
void delayms(uint t){
	uint i,j;
	for(i=0;i<t;i++)
		for(j=0;j<120;j++);
}
//自检
void self_check(){
	//从0-9依次点亮	
	P1=0x00;
	P0=0x00;
	delayms(1000);	
}
//显示选择时间
void display_time(uint time){
	uint bai,shi,ge,i;
	uint tp[3];

	if(time>=100){
	 	bai=time/100;
		shi=(time-bai*100)/10;
		ge=time%10;
	}else if(time>=10&&time<100){
		bai=10;
		shi=time/10;
		ge=time%10;
	}else if(time>=0&&time<10){
		bai=10;
		shi=10;
		ge=time;
	}

	tp[0]=num[bai];
	tp[1]=num[shi];
	tp[2]=num[ge];

	for(i=0;i<3;i++){
		P1=0xff;
		P0=0xff;

	 	P1=WELA[1+i];
		P0=tp[i];
		delayms(1);

		P1=0xff;
		P0=0xff;
	}
}
//动态显示温度
void display_num1(float tp){
	uint x,shi,ge,d,i,t[3];  
	
	x=(int)tp;
	shi=x/10;
	ge=x%10;
	d=(tp-x)*10;
	
	t[0]=num[shi];
	t[1]=num_point[ge];
	t[2]=num[d];

	ET0=0;
	for(i=0;i<3;i++){
	 	P1=0xff;
		P0=0xff;

		P1=WELA[i];
		P0=t[i];
		delayms(1);

		P1=0xff;
		P0=0xff;
	}
	ET0=1;
}


//动态显示转速
void display_round(uint pp){
	uint round;
	uint qian,bai,shi,ge,i;
	uint r[4];
	round=pp*600;

	if(round>1000){
		qian=round/1000;
		bai=(round-qian*1000)/100;
		shi=(round-qian*1000-bai*100)/10;
		ge=round%10;
	}else{
		qian=10;
		bai=round/100;
		shi=(round-bai*100)/10;
		ge=round%10;
	}

	r[0]=num[qian];
	r[1]=num[bai];
	r[2]=num[shi];
	r[3]=num[ge];

	round_t=0;


	while(round_t<20){
		for(i=0;i<4;i++){
	 		P1=0xff;
			P0=0xff;

			P1=WELA[i];
			P0=r[i];
			delayms(1);

		}

		if(num_0<pp){
		 	mos=1;
		}else{
		 	mos=0;
		}
	} 
}

/****扫描键盘选择******/
uint scan_clock(){
 	uint choose_1,choose_2,choose_3,choose_code;
	choose_code=0;

	choose_1=key_1;
	if(choose_1==0){
		do{
			choose_1=key_1;
			display_time(time_num);

			if(num_0<pp){
		 		mos=1;
			}else{
			 	mos=0;
			}
		}while(choose_1==0);
		//减时间
		if(time_num>10){
		 	time_num=time_num-10;
		}
	}
	choose_2=key_2;
	if(choose_2==0){
		do{
			choose_2=key_2;
			display_time(time_num);

			if(num_0<pp){
		 		mos=1;
			}else{
		 		mos=0;
			}
		}while(choose_2==0);
		//加时间
		if(time_num<1000){
		 	time_num=time_num+10;
		}

	}	   
	choose_3=key_3;
	if(choose_3==0){
		do{
			choose_2=key_2;
			display_time(time_num);
		}while(choose_2==0);
		//确认选择
		choose_code=1;
	}
	return choose_code;
}
/*********智能*********/
void intelligent(uint il){
	if(il==1){
		if(t<15){
		 	pp=0;
		}else if(t<18&&t>=15){
		 	pp=1;
		}else if(t<21&&t>=18){
		 	pp=3;
		}else if(t<24&&t>=21){
		 	pp=5;
		}else if(t<27&&t>=24){
		 	pp=7;
		}else if(t<30&&t>=27){
		 	pp=8;
		}else if(t>=30){
		 	pp=9;
		}
	}
}
//定时
void clock(uint time){
	uint temp_t;
	round_t=0;
	il=1;
		

	while(round_t<=time){		
		temp_t=(time-round_t)/10;
		intelligent(il);
		//倒计时
		display_time(temp_t);

		if(num_0<pp){
		 	mos=1;
		}else{
		 	mos=0;
		}
	}
	pp=0;
	il=0;
	STANDBY=1;
}
//选择定时
void choose_clock(){
	uint choose=0,tim;
	do{
		display_time(time_num);
		choose=scan_clock();

		if(num_0<pp){
		 	mos=1;
		}else{
		 	mos=0;
		}
	}while(choose==0);
	tim=time_num*10;
	clock(tim);
}



/******键盘扫描******/
void scan_2(){
 	uint scan_1,scan_2,scan_3,scan_4;
   
	scan_1=key_1;
	//减档
	if(scan_1==0){
	 	delayms(5);
		do{
		 	scan_1=key_1;
			display_num1(t);

			if(num_0<pp){
			 	mos=1;
			}else{
			 	mos=0;
			}
		}while(scan_1==0);
		il=0;
		if(pp!=0){
			STANDBY=0;

			if(pp>0&&pp<=3){
			 	pp=3;
			}else if(pp>3&&pp<=6){
			 	pp=6;
			}else if(pp>6&&pp<=10){
			 	pp=9;
			}
			pp=pp-3;

		}else{
		 	STANDBY=1;
		}
	}  
	//加档
	scan_2=key_2;
	if(scan_2==0){
	 	delayms(5);
		do{
		 	scan_2=key_2;
			display_num1(t);

			if(num_0<pp){
			 	mos=1;
			}else{
			 	mos=0;
			}
		}while(scan_2==0);
		il=0;
		if(pp!=9){
			if(pp>=0&&pp<3){
			 	pp=0;
			}else if(pp>=3&&pp<6){
			 	pp=3;
			}else if(pp>=6&&pp<10){
			 	pp=6;
			}
			pp=pp+3;
			STANDBY=0;
		}
	}  
	//显示转速
	scan_3=key_3;
	if(scan_3==0){
	
		round_t=0;
		do{
			scan_3=key_3;
			display_num1(t);

			if(num_0<pp){
		 		mos=1;
			}else{
		 		mos=0;
			}
		}while(scan_3==0);


		if(round_t<=12){
			display_round(pp); 
			STANDBY=0;
		}else{
			choose_clock();
		}
	}  
	//智能
	scan_4=key_4;
	if(scan_4==0){
	 	delayms(5);
		do{
		 	scan_4=key_4;
			display_num1(t);

			if(num_0<pp){
		 		mos=1;
			}else{
		 		mos=0;
			}

		}while(scan_4==0);
		il=1;
		STANDBY=0;
		
	}  
}

/**********嘴吹启动*******/
void month(){
	uint lm358;

	if(pp==0){
		lm358=LM358;
		if(lm358==1){
			delayms(100);
			lm358=LM358;
			if(lm358==1){
		 		il=1;
				STANDBY=0;
			}
		}
	} 
}


void main(){

	num_0=0;
	round_t=0;
	time_num=10;

	pp=0;

	TMOD=0x01;
	TH0=(65536-10000)/256;
	TL0=(65536-10000)%256;
	EA=1;
	ET0=1;
	TR0=1;

	STANDBY=0;
	
	mos=0;
	LM358=0;

	self_check();
	
	il=0;
	t=0;
	while(1){
	   
		tmpchange();
		t=tmp();
		while(t>=80.5){
		 	tmpchange();
			t=tmp();
		}
		display_num1(t);
		intelligent(il);
		month();
		scan_2();

		
		if(num_0<pp){
		 	mos=1;
		}else{
		 	mos=0;
		}

		while(STANDBY==1){
			month();
			scan_2();
		}
		
	}
}
//10ms				 
void T0_time() interrupt 1{
	TH0=(65536-10000)/256;
	TL0=(65536-10000)%256;
	num_0++;

	if(num_0>=10){
	 	num_0=0;
		round_t++;
		if(round_t>=1000){
		 	round_t=0;
		}
	}
}