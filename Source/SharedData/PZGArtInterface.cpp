//
//  PZGArtInterface.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGArtInterface.h"
#include "constants.h"

PZGArtInterface::PZGArtInterface(){
    
}

PZGArtInterface::~PZGArtInterface(){
    
}


PZGArtInterface* PZGArtInterface::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtInterface *obj = new PZGArtInterface();
    
    obj->className = dictionary->valueForKey("className");
    obj->key = (ax::__String*)dictionary->valueForKey("key");

    obj->subkey = (ax::__String*)dictionary->valueForKey("subkey");

    obj->name = (ax::__String*)dictionary->valueForKey("name");

    obj->no_of_sprites = dictionary->valueForKey("no_of_sprites")->intValue();
    obj->index = dictionary->valueForKey("index")->intValue();

    
    obj->bouncing = dictionary->valueForKey("bouncing")->boolValue();
    obj->position_lock = dictionary->valueForKey("position lock")->boolValue();
    
    obj->position_x = dictionary->valueForKey("position X")->floatValue();
    obj->position_y = dictionary->valueForKey("position Y")->floatValue();

    obj->scale_x = dictionary->valueForKey("scale_x")->floatValue();
    obj->scale_y = dictionary->valueForKey("scale_y")->floatValue();
    
    obj->alignment = (PZGInterfaceAlign)dictionary->valueForKey("alignment")->intValue();
    
    obj->rotation = dictionary->valueForKey("rotation")->floatValue();

    obj->type = (ax::__String*)dictionary->valueForKey("type");
    
    return obj;
}

ax::Vec2 PZGArtInterface::getPosition(){
    ax::Size size = ax::Director::getInstance()->getWinSize();
    ax::Size baseSize;
    ax::Vec2 p;
    
    if (size.width > size.height) {
        baseSize = ax::Size(960 * kDeviceScale(), 640 * kDeviceScale());
    }else{
        baseSize = ax::Size(640 * kDeviceScale(), 960 * kDeviceScale());
    }
    
    switch (alignment) {
        case PZGInterfaceAlignNone:
            p = ccp(position_x * size.width,
                    position_y * size.height);
            break;
            
        case PZGInterfaceAlignUpLeft:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(p.x, size.height - (baseSize.height - p.y) );
            break;
            
        case PZGInterfaceAlignUp:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(position_x*size.width, size.height - (baseSize.height - p.y) );
            break;
            
        case PZGInterfaceAlignUpRight:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(size.width - (baseSize.width - p.x), size.height - (baseSize.height - p.y) );
            break;
            
        case PZGInterfaceAlignLeft:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(p.x, position_y*size.height );
            break;
            
        case PZGInterfaceAlignCenter:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(p.x - baseSize.width*0.5, p.y - baseSize.height*0.5 );
            p = ccp(p.x + size.width*0.5, p.y + size.height*0.5 );
            break;
            
        case PZGInterfaceAlignRight:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(size.width - (baseSize.width - p.x), position_y*size.height );
            break;
            
        case PZGInterfaceAlignDownLeft:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(p.x, p.y );
            break;
            
        case PZGInterfaceAlignDown:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(position_x*size.width, p.y );
            break;
            
        case PZGInterfaceAlignDownRight:
            p = ccp(position_x*baseSize.width, position_y*baseSize.height);
            p = ccp(size.width - (baseSize.width - p.x), p.y );
            break;
            
        default:
            break;
    }
    
    return p;
}
