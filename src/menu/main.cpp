#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <vector>
#include <string>
#include <functional>

#define EPD_BUSY 17
#define EPD_RST 16
#define EPD_DC 4
#define EPD_CS 5

#define BTN_UP 33
#define BTN_DOWN 32
#define BTN_ENTER 27
#define BTN_BACK 26

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

using string = std::string;
using Strings = std::vector<string>;
using Action = std::function<void()>;

struct Menu;

struct MenuItem {
    string text;
    Action action;
    Menu* submenu;
    MenuItem(const string& t="", Action a=nullptr, Menu* s=nullptr):text(t),action(a),submenu(s){}
};

struct Menu {
    std::vector<MenuItem> items;
    int selected_index;
    Menu* parent;
    Menu(const std::vector<MenuItem>& it={}):items(it),selected_index(0),parent(nullptr){}
    void move_up(){if(items.size())selected_index=selected_index>0?selected_index-1:0;}
    void move_down(){if(items.size())selected_index=selected_index<(int)items.size()-1?selected_index+1:items.size()-1;}
    Menu* select(){if(items.empty())return this;MenuItem& it=items[selected_index];if(it.submenu){it.submenu->parent=this;return it.submenu;}if(it.action)it.action();return this;}
};

unsigned long debounce_ms=180;
unsigned long last_up=0;
unsigned long last_down=0;
unsigned long last_enter=0;
unsigned long last_back=0;

int16_t line_h=14;
int16_t screen_w=0;
int16_t screen_h=0;

string repeat_text(const string& s,int n){
    string out;
    for(int i=0;i<n;i++)out+=s;
    return out;
}

Strings wrap_text(const string& text,int16_t maxw){
    Strings out;
    string cur;
    size_t i=0;
    while(i<text.size()){
        while(i<text.size()&&text[i]==' ')i++;
        if(i>=text.size())break;
        size_t j=i;
        while(j<text.size()&&text[j]!=' ')j++;
        string word=text.substr(i,j-i);
        string test=cur.empty()?word:cur+" "+word;
        int16_t bx,by;
        uint16_t bw,bh;
        display.getTextBounds(test.c_str(),0,0,&bx,&by,&bw,&bh);
        if(bw<=maxw){
            cur=test;
        }else{
            if(!cur.empty())out.push_back(cur);
            cur=word;
        }
        i=j;
    }
    if(!cur.empty())out.push_back(cur);
    return out;
}

void render_menu(Menu* menu){
    display.fillScreen(GxEPD_WHITE);
    int16_t y=12;
    for(size_t i=0;i<menu->items.size();i++){
        Strings lines=wrap_text(menu->items[i].text,screen_w-10);
        for(const string& line:lines){
            int16_t bx,by;
            uint16_t bw,bh;
            display.getTextBounds(line.c_str(),0,0,&bx,&by,&bw,&bh);
            if((int)i==menu->selected_index){
                display.fillRect(2,y-2,bw+6,bh+4,GxEPD_BLACK);
                display.setTextColor(GxEPD_WHITE);
            }else display.setTextColor(GxEPD_BLACK);
            display.setCursor(6,y+bh-2);
            display.print(line.c_str());
            y+=line_h;
        }
        y+=4;
    }
    display.display();
}

void display_page(const string& title,const string& content){
    display.fillScreen(GxEPD_WHITE);
    int16_t y=12;
    Strings t=wrap_text(title,screen_w-10);
    for(const string& line:t){
        int16_t bx,by;
        uint16_t bw,bh;
        display.getTextBounds(line.c_str(),0,0,&bx,&by,&bw,&bh);
        display.setCursor(6,y+bh-2);
        display.print(line.c_str());
        y+=line_h;
    }
    y+=6;
    Strings body=wrap_text(content,screen_w-10);
    int max_lines=(screen_h-y)/line_h;
    for(int i=0;i<max_lines && i<(int)body.size();i++){
        int16_t bx,by;
        uint16_t bw,bh;
        display.getTextBounds(body[i].c_str(),0,0,&bx,&by,&bw,&bh);
        display.setCursor(6,y+bh-2);
        display.print(body[i].c_str());
        y+=line_h;
    }
    display.display();
    bool wait=true;
    while(wait){
        if(digitalRead(BTN_ENTER)==LOW&&millis()-last_enter>debounce_ms){last_enter=millis();wait=false;}
        if(digitalRead(BTN_BACK)==LOW&&millis()-last_back>debounce_ms){last_back=millis();wait=false;}
        delay(40);
    }
}

Menu main_menu;
Menu books_menu;
Menu settings_menu;

void setup_menus(){
    std::vector<MenuItem> books;
    books.push_back(MenuItem("The Hobbit",[](){display_page("The Hobbit",repeat_text("This is test content for The Hobbit. ",8));},nullptr));
    books.push_back(MenuItem("1984",[](){display_page("1984",repeat_text("This is test content for 1984. ",8));},nullptr));
    books.push_back(MenuItem("Brave New World",[](){display_page("Brave New World",repeat_text("This is test content for Brave New World. ",8));},nullptr));
    books_menu=Menu(books);

    std::vector<MenuItem> settings;
    settings.push_back(MenuItem("Font Size",[](){display_page("Settings","Font size example");},nullptr));
    settings.push_back(MenuItem("Theme",[](){display_page("Settings","Theme example");},nullptr));
    settings_menu=Menu(settings);

    std::vector<MenuItem> main;
    main.push_back(MenuItem("Open a Book",nullptr,&books_menu));
    main.push_back(MenuItem("Settings",nullptr,&settings_menu));
    main.push_back(MenuItem("About",[](){display_page("About","Nested menu demo on e-ink.");},nullptr));
    main.push_back(MenuItem("Exit",[](){display_page("Exit","Goodbye.");while(true)delay(1000);},nullptr));
    main_menu=Menu(main);
}

void read_buttons(bool& up,bool& down,bool& enter,bool& back){
    up=down=enter=back=false;
    if(digitalRead(BTN_UP)==LOW&&millis()-last_up>debounce_ms){last_up=millis();up=true;}
    if(digitalRead(BTN_DOWN)==LOW&&millis()-last_down>debounce_ms){last_down=millis();down=true;}
    if(digitalRead(BTN_ENTER)==LOW&&millis()-last_enter>debounce_ms){last_enter=millis();enter=true;}
    if(digitalRead(BTN_BACK)==LOW&&millis()-last_back>debounce_ms){last_back=millis();back=true;}
}

void setup(){
    pinMode(BTN_UP,INPUT_PULLUP);
    pinMode(BTN_DOWN,INPUT_PULLUP);
    pinMode(BTN_ENTER,INPUT_PULLUP);
    pinMode(BTN_BACK,INPUT_PULLUP);
    display.init();
    display.setRotation(0);
    display.setFont(&FreeMonoBold12pt7b);
    int16_t bx,by;
    uint16_t bw,bh;
    display.getTextBounds("A",0,0,&bx,&by,&bw,&bh);
    line_h=bh+4;
    screen_w=display.width();
    screen_h=display.height();
    setup_menus();
    render_menu(&main_menu);
}

void loop(){
    static Menu* current=&main_menu;
    bool up,down,enter,back;
    read_buttons(up,down,enter,back);
    if(up){current->move_up();render_menu(current);}
    if(down){current->move_down();render_menu(current);}
    if(enter){
        Menu* next=current->select();
        if(next!=current){current=next;render_menu(current);}
        else render_menu(current);
    }
    if(back){
        if(current->parent){current=current->parent;render_menu(current);}
    }
    delay(60);
}
