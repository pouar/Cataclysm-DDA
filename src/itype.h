#ifndef ITYPE_H
#define ITYPE_H

#include "color.h"
#include "enums.h"
#include "iuse.h"
#include "pldata.h"
#include "bodypart.h"
#include "skill.h"
#include "bionics.h"
#include "rng.h"
#include "material.h"
#include "mtype.h"
#include "translations.h"

#include <string>
#include <vector>
#include <set>
#include <bitset>
#include <memory>

typedef std::string itype_id;

// see item.h
class item_category;
struct recipe;
struct itype;

typedef std::string ammotype;

enum software_type {
    SW_USELESS,
    SW_HACKING,
    SW_MEDICAL,
    SW_SCIENCE,
    SW_DATA
};

enum bigness_property_aspect {
    BIGNESS_ENGINE_DISPLACEMENT, // combustion engine CC displacement
    BIGNESS_WHEEL_DIAMETER,      // wheel size in inches, including tire
};

// Returns the name of a category of ammo (e.g. "shot")
std::string ammo_name(ammotype t);
// Returns the default ammo for a category of ammo (e.g. ""00_shot"")
itype_id default_ammo(ammotype guntype);

struct explosion_data {
    // Those 4 values are forwarded to game::explosion.
    int power    = -1;
    int shrapnel = 0;
    bool fire    = false;
    bool blast   = true;
};

struct islot_container {
    /**
     * Volume, scaled by the default-stack size of the item that is contained in this container.
     */
    int contains = 0;
    /**
     * Can be resealed.
     */
    bool seals = false;
    /**
     * Can hold liquids.
     */
    bool watertight = false;
    /**
     * Contents do not spoil.
     */
    bool preserves = false;
    /**
     * Volume of the item does not include volume of the content.
     */
    bool rigid = false;
};

struct islot_armor {
    /**
     * Bitfield of enum body_part
     * TODO: document me.
     */
    std::bitset<num_bp> covers;
    /**
     * Bitfield of enum body_part
     * TODO: document me.
     */
    std::bitset<num_bp> sided;
    /**
     * How much this item encumbers the player.
     */
    signed char encumber = 0;
    /**
     * Percentage of the body part area that this item covers.
     * This determines how likely it is to hit the item instead of the player.
     */
    unsigned char coverage = 0;
    /**
     * TODO: document me.
     */
    unsigned char thickness = 0;
    /**
     * Resistance to environmental effects.
     */
    unsigned char env_resist = 0;
    /**
     * How much warmth this item provides.
     */
    signed char warmth = 0;
    /**
     * How much storage this items provides when worn.
     */
    unsigned char storage = 0;
    /**
     * Whether this is a power armor item.
     */
    bool power_armor = false;
};

struct islot_book {
    /**
     * Which skill it upgrades, if any. Can be NULL.
     * TODO: this should be a pointer to const
     */
    const Skill* skill = nullptr;
    /**
     * The skill level the book provides.
     */
    int level = 0;
    /**
     * The skill level required to understand it.
     */
    int req = 0;
    /**
     * How fun reading this is, can be negative.
     */
    int fun = 0;
    /**
     * Intelligence required to read it.
     */
    int intel = 0;
    /**
     * How long, in 10-turns (aka minutes), it takes to read.
     * "To read" means getting 1 skill point, not all of them.
     */
    int time = 0;
    /**
     * Fun books have chapters; after all are read, the book is less fun.
     */
    int chapters = 0;
    /**
     * What recipes can be learned from this book.
     * Key is the recipe, value is skill level (of the main skill of the recipes) that is required
     * to learn the recipe.
     */
    std::map<const recipe *, int> recipes;
    /**
     * Special effects that can happen after the item has been read. May be empty.
     */
    std::vector<use_function> use_methods;
};

/**
 * Common data for ranged things: guns, gunmods and ammo.
 * The values of the gun itself, its mods and its current ammo (optional) are usually summed
 * up in the item class and the sum is used.
 */
struct common_ranged_data {
    /**
     * Armor-pierce bonus from gun.
     */
    int pierce = 0;
    /**
     * Range bonus from gun.
     */
    int range = 0;
    /**
     * Damage bonus from gun.
     */
    int damage = 0;
    /**
     * Dispersion "bonus" from gun.
     */
    int dispersion = 0;
    /**
     * Recoil "bonus" from gun.
     */
    int recoil = 0;
};

/**
 * Common data for things that affect firing: guns and gunmods.
 * The values of the gun itself and its mods are usually summed up in the item class
 * and the sum is used.
 */
struct common_firing_data : public common_ranged_data {
    /**
     * TODO: this needs documentation, who knows what it is?
     * A value of -1 in gunmods means it's ignored.
     */
    int sight_dispersion = 0;
    /**
     * TODO: this needs documentation, who knows what it is?
     * A value of -1 in gunmods means it's ignored.
     */
    int aim_speed = 0;
    /**
     * Burst size.
     */
    int burst = 0;
    /**
     * Clip size. Note that on some gunmods it means relative (in percent) of the
     * guns main magazine.
     */
    int clip = 0;
    /**
     * TODO: document me
     */
    int loudness = 0;
};

// TODO: this shares a lot with the ammo item type, merge into a separate slot type?
struct islot_gun : public common_firing_data {
    /**
     * What type of ammo this gun uses.
     */
    std::string ammo;
    /**
     * What skill this gun uses.
     * TODO: This is also indicates the type of gun (handgun/rifle/etc.) - that
     * should probably be made explicit.
     */
    const Skill* skill_used = nullptr;
    /**
     * Gun durability, affects gun being damaged during shooting.
     */
    int durability = 0;
    /**
     * Reload time.
     */
    int reload_time = 0;
    /**
     * Effects that are applied to the ammo when fired.
     */
    std::set<std::string> ammo_effects;
    /**
     * Location for gun mods.
     * Key is the location (untranslated!), value is the number of mods
     * that the location can have. The value should be > 0.
     */
    std::map<std::string, int> valid_mod_locations;
    /**
     * If this uses UPS charges, how many (per shoot), 0 for no UPS charges at all.
     */
    int ups_charges = 0;
};

struct islot_gunmod : public common_firing_data {
    /**
     * TODO: document me
     */
    int req_skill = 0;
    /**
     * TODO: document me
     * TODO: this should be a pointer to const Skill.
     */
    const Skill* skill_used = nullptr;
    /**
     * TODO: document me
     */
    std::string newtype;
    /**
     * TODO: document me
     */
    std::set<std::string> acceptible_ammo_types;
    /**
     * TODO: document me
     */
    bool used_on_pistol = false;
    /**
     * TODO: document me
     */
    bool used_on_shotgun = false;
    /**
     * TODO: document me
     */
    bool used_on_smg = false;
    /**
     * TODO: document me
     */
    bool used_on_rifle = false;
    /**
     * TODO: document me
     */
    bool used_on_bow = false;
    /**
     * TODO: document me
     */
    bool used_on_crossbow = false;
    /**
     * TODO: document me
     */
    bool used_on_launcher = false;
    /**
     * TODO: document me
     */
    std::string location;
};

struct islot_ammo : public common_ranged_data {
    /**
     * Ammo type, basically the "form" of the the ammo that fits into the gun/tool.
     * This is an id, it can be looked up in the @ref ammunition_type class.
     */
    std::string type;
    /**
     * Type id of casings, can be "NULL" for no casings at all.
     */
    std::string casing;
    /**
     * Default charges.
     */
    long def_charges = 0;
    /**
     * TODO: document me.
     */
    std::set<std::string> ammo_effects;

    islot_ammo() : casing {"NULL"} { }
};

struct islot_variable_bigness {
    /**
     * Minimal value of the bigness value of items of this type.
     */
    int min_bigness = 0;
    /**
     * Maximal value of the bigness value of items of this type.
     */
    int max_bigness = 0;
    /**
     * What the bigness actually represent see @ref bigness_property_aspect
     */
    bigness_property_aspect bigness_aspect = BIGNESS_ENGINE_DISPLACEMENT;
};

struct islot_bionic {
    /**
     * Arbitrary difficulty scale, see bionics.cpp for its usage.
     */
    int difficulty = 0;
    /**
     * Id of the bionic, see @ref bionics.
     */
    std::string bionic_id;
};

struct islot_software {
    /**
     * Type of software, see enum.
     */
    software_type swtype = SW_USELESS;
};

// Data used when spawning items, should be obsoleted by the spawn system, but
// is still used at several places and makes it easier when it applies to all new items of a type.
struct islot_spawn {
    std::string default_container; // The container it comes in
    std::vector<long> rand_charges;

    islot_spawn() : default_container {"null"} { }
};

struct itype {
    itype_id id; // unique string identifier for this item,
    // can be used as lookup key in master itype map
    // Used for save files; aligns to itype_id above.
    unsigned int  price; // Its value

    /**
     * Slots for various item type properties. Each slot may contain a valid pointer or null, check
     * this before using it.
     */
    /*@{*/
    std::unique_ptr<islot_container> container;
    std::unique_ptr<islot_armor> armor;
    std::unique_ptr<islot_book> book;
    std::unique_ptr<islot_gun> gun;
    std::unique_ptr<islot_gunmod> gunmod;
    std::unique_ptr<islot_variable_bigness> variable_bigness;
    std::unique_ptr<islot_bionic> bionic;
    std::unique_ptr<islot_software> software;
    std::unique_ptr<islot_spawn> spawn;
    std::unique_ptr<islot_ammo> ammo;
    /*@}*/

protected:
    friend class Item_factory;
    // private because is should only be accessed through itype::nname!
    // name and name_plural are not translated automatically
    // nname() is used for display purposes
    std::string name;        // Proper name, singular form, in American English.
    std::string name_plural; // name, plural form, in American English.
public:
    std::string description; // Flavor text

    char sym;       // Symbol on the map
    nc_color color; // Color on the map (color.h)

    // What we're made of (material names). .size() == made of nothing.
    // MATERIALS WORK IN PROGRESS.
    std::vector<std::string> materials;

    phase_id phase; //e.g. solid, liquid, gas

    unsigned int volume; // Space taken up by this item
    int stack_size;      // How many things make up the above-defined volume (eg. 100 aspirin = 1 volume)
    unsigned int weight; // Weight in grams. Assumes positive weight. No helium, guys!
    std::map<std::string, int> qualities; //Tool quality indicators
    std::map<std::string, std::string> properties;

    // Explosion that happens when the item is set on fire
    explosion_data explosion_on_fire_data;
    bool explode_in_fire() const
    {
        return explosion_on_fire_data.power >= 0;
    }

    signed int melee_dam; // Bonus for melee damage; may be a penalty
    signed int melee_cut; // Cutting damage in melee
    int peecap; 
    signed int m_to_hit;  // To-hit bonus for melee combat; -5 to 5 is reasonable

    std::set<std::string> item_tags;
    std::set<std::string> techniques;

    unsigned int light_emission;   // Exactly the same as item_tags LIGHT_*, this is for lightmap.

    const item_category *category; // category pointer or NULL for automatic selection

    std::string snippet_category;

    virtual std::string get_item_type_string() const
    {
        if( container ) {
            return "CONTAINER";
        } else if( armor ) {
            return "ARMOR";
        } else if( book ) {
            return "BOOK";
        } else if( gun ) {
            return "GUN";
        } else if( variable_bigness ) {
            return "VEHICLE_PART";
        } else if( bionic ) {
            return "BIONIC";
        } else if( ammo ) {
            return "AMMO";
        }
        return "misc";
    }

    // Returns the name of the item type in the correct language and with respect to its grammatical number,
    // based on quantity (example: item type “anvil”, nname(4) would return “anvils” (as in “4 anvils”).
    virtual std::string nname(unsigned int quantity) const
    {
        return ngettext(name.c_str(), name_plural.c_str(), quantity);
    }

    virtual bool is_food() const
    {
        return false;
    }
    virtual bool is_tool() const
    {
        return false;
    }
    virtual bool is_artifact() const
    {
        return false;
    }
    virtual bool count_by_charges() const
    {
        if( ammo ) {
            return true;
        }
        return false;
    }
    virtual int charges_to_use() const
    {
        return 1;
    }
    virtual int maximum_charges() const
    {
        return 1;
    }

    bool has_use() const;
    bool can_use( const std::string &iuse_name ) const;
    const use_function *get_use( const std::string &iuse_name ) const;
    int invoke( player *p, item *it, bool active, point pos );

    std::vector<use_function> use_methods;// Special effects of use

    itype() : id("null"), price(0), name("none"), name_plural("none"), description(), sym('#'),
        color(c_white), phase(SOLID), volume(0), stack_size(0),
        weight(0), qualities(),
        melee_dam(0), melee_cut(0), m_to_hit(0), item_tags(), techniques(), light_emission(),
        category(NULL)
    {}

    itype(std::string pid, unsigned int pprice, std::string pname, std::string pname_plural,
          std::string pdes, char psym, nc_color pcolor, std::vector<std::string> pmaterials,
          phase_id pphase, unsigned int pvolume, unsigned int pweight, signed int pmelee_dam,
          signed int pmelee_cut, signed int pm_to_hit) : id(pid), price(pprice), name(pname),
        name_plural(pname_plural), description(pdes), sym(psym), color(pcolor), materials(pmaterials),
        phase(pphase), volume(pvolume), stack_size(0), weight(pweight),
        qualities(), melee_dam(pmelee_dam),
        melee_cut(pmelee_cut), m_to_hit(pm_to_hit), item_tags(), techniques(), light_emission(),
        category(NULL) { }

    virtual ~itype() {}
};

// Includes food drink and drugs
struct it_comest : public virtual itype {
    signed int quench;     // Many things make you thirstier!
    unsigned int nutr;     // Nutrition imparted
    /**
     * How long it takes to spoil (turns), rotten food is handled differently
     * (chance of bad thinks happen when eating etc).
     * If 0, the food never spoils.
     */
    int spoils;
    unsigned int addict;   // Addictiveness potential
    long def_charges;  // Defaults # of charges (drugs, loaf of bread? etc)
    signed int stim;
    signed int healthy;
    unsigned int brewtime; // How long it takes for a brew to ferment.
    std::string comesttype; //FOOD, DRINK, MED

    signed int fun;    // How fun its use is

    unsigned int grow; //time it takes for a seed to grow (in days, based of off a season length of 91)
    itype_id tool;      // Tool needed to consume (e.g. lighter for cigarettes)

    virtual bool is_food() const
    {
        return true;
    }
    virtual std::string get_item_type_string() const
    {
        return "FOOD";
    }

    virtual bool count_by_charges() const
    {
        if (phase == LIQUID) {
            return true;
        } else {
            return def_charges > 1 ;
        }
    }

    add_type add; // Effects of addiction

    it_comest(): itype(), quench(0), nutr(0), def_charges(0), stim(0), healthy(0),
        brewtime(0), comesttype(), fun(0), tool()
    {
    }
};

struct it_tool : public virtual itype {
    std::string ammo;
    long max_charges = 0;
    long def_charges = 0;
    unsigned char charges_per_use = 0;
    unsigned char turns_per_charge = 0;
    std::string revert_to;
    std::string subtype;

    virtual bool is_tool() const
    {
        return true;
    }
    virtual bool is_artifact() const
    {
        return false;
    }
    virtual std::string get_item_type_string() const
    {
        return "TOOL";
    }
    int charges_to_use() const
    {
        return charges_per_use;
    }
    int maximum_charges() const
    {
        return max_charges;
    }
};

#endif

