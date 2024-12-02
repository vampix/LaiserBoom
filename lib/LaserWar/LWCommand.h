#ifndef LWCommand_h
#define LWCommand_h
#include <Arduino.h>
#include <Constants.h>

class LWCommand

#ifdef Printable_h
: public Printable 
#endif

{
private:
    unsigned char group;
    unsigned char data;
public:
    LWCommand(){}
    LWCommand(LwSetting group, unsigned char data){
        this->setGroup(group);
        this->setData(data);
    }
    bool load(unsigned long command){
        if ((command & 0xff) != LW_CMD_END) return false;
        this->data = (command >> 8) & 0xff;
        this->group = (command >> 16) & 0xff;
        return true;
    }
    LwSetting getGroup(){ return (LwSetting)this->group; }
    unsigned char getData(){ return this->data; }
    void setGroup(LwSetting group){ this->group = group; }
    void setData(unsigned char data){ this->data = data; }
    void setCommand(LwSetting group, unsigned char data){
        this->setGroup(group);
        this->setData(data);
    }
    unsigned long getCommand(){
        return ((((unsigned long)this->group << 8) + (unsigned long)this->data) << 8) + (unsigned long)LW_CMD_END;
    }

#ifdef Printable_h
    size_t printTo(Print& p) const {
        size_t r = 0;

        if (this->group == LwSetting::ApplyPreset){
            r += p.print("Apply preset ");
            r += p.print(preset_list[this->data]);
            return r;
        }
        
        if (this->group == LwSetting::AdminCommand){
            for (unsigned char i = 0; i < 26; i++){
                if (commands83[i].code == this->data){
                    r += p.print(commands83[i].description);
                    return r;
                }
            }
            return r;
        }

        for (unsigned char i = 0; i < 5; i++){
            if (colorCommands[i].code == this->group){
                r += p.print(colorCommands[i].description);
                r += p.print(color_list[this->data]);
                return r;
            }
        }

        for (unsigned char i = 0; i < 11; i++){
            if (dataCommands[i].code == this->group){
                r += p.print(dataCommands[i].description);
                r += p.print(this->data, DEC);
                return r;
            }
        }

        return r;
    }
#endif

};

#endif