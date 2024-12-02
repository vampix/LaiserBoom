#ifndef LWShoot_h
#define LWShoot_h

#include <Arduino.h>
#include <Constants.h>

class LWShoot

#ifdef Printable_h
: public Printable
#endif

{
    private:
        unsigned long color;
        unsigned char damage, id;
    public:
        LWShoot(){}
        LWShoot(LWColor color, unsigned char damage, unsigned char id){
            this->setShoot(color, damage, id);
        }
        bool load(unsigned long signal){
            if (signal >= 0x1FFF || signal <= 0x40) return false;
            this->damage = dmg_list[signal & 0b1111];
            this->color = (signal >> 4) & 0b11;
            this->id = (signal >> 6) & 0b1111111;
            return true;
        }

        unsigned long getColor(){ return this->color; }
        unsigned char getDamage() { return this->damage; }
        unsigned char getId(){ return this->id; }
        unsigned long getCommand() {
            return (((this->id << 2) + this->color) << 4) + this->damage;
        }
        
        void setColor(LWColor color){ this->color = color; }
        void setId(unsigned char id){ this->id = id; }
        void setDamage(unsigned char damage){
            for (unsigned char i = 0; i < 16; i++){
                if (dmg_list[i] == damage){
                    this->damage = i;
                    return;
                }
            }
        }
        void setShoot(LWColor color, unsigned char damage, unsigned char id){
            this->setColor(color);
            this->setDamage(damage);
            this->setId(id);
        }

 #ifdef Printable_h
        size_t printTo(Print& p) const {
            size_t r = 0;
            r += p.print("Shoot. id=");
            r += p.print(this->id);
            r += p.print("; color=");
            r += p.print(color_list[this->color]);
            r += p.print("; damage=");
            r += p.print(this->damage);
            return r;
        }
#endif
};
#endif