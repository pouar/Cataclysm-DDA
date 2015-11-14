#ifndef CRAFTING_H
#define CRAFTING_H

#include "item.h"         // item
#include "requirements.h" // requirement_data
#include "cursesdef.h"    // WINDOW
#include "string_id.h"

#include <string>
#include <vector>
#include <map>
#include <list>

class JsonObject;
class Skill;
using skill_id = string_id<Skill>;
class inventory;
class player;
struct recipe;

enum body_part : int; // From bodypart.h
typedef int nc_color; // From color.h

using itype_id     = std::string; // From itype.h

struct byproduct {
    itype_id result;
    int charges_mult;
    int amount;

    byproduct() : byproduct("null") {}

    byproduct(itype_id res, int mult = 1, int amnt = 1)
        : result(res), charges_mult(mult), amount(amnt)
    {
    }
};

struct recipe {
    std::string ident;
    int id;
    itype_id result;
    int time; // in movement points (100 per turn)
    int difficulty;
    requirement_data requirements;
    std::vector<byproduct> byproducts;
    std::string cat;
    bool contained; // Does the item spawn contained?
    std::string subcat;
    skill_id skill_used;
    std::map<skill_id, int> required_skills;
    bool reversible; // can the item be disassembled?
    bool autolearn; // do we learn it just by leveling skills?
    int learn_by_disassembly; // what level (if any) do we learn it by disassembly?

    // maximum achievable time reduction, as percentage of the original time.
    // if zero then the recipe has no batch crafting time reduction.
    double batch_rscale;
    int batch_rsize; // minimum batch size to needed to reach batch_rscale
    int result_mult; // used by certain batch recipes that create more than one stack of the result

    // only used during loading json data: book_id is the id of an book item, other stuff is copied
    // into @ref islot_book::recipes.
    struct bookdata_t {
        std::string book_id;
        int skill_level;
        std::string recipe_name;
        bool hidden;
    };
    std::vector<bookdata_t> booksets;

    //Create a string list to describe the skill requirements fir this recipe
    // Format: skill_name(amount), skill_name(amount)
    std::string required_skills_string() const;

    recipe();

    // Create an item instance as if the recipe was just finished,
    // Contain charges multiplier
    item create_result() const;
    std::vector<item> create_results(int batch = 1) const;

    // Create byproduct instances as if the recipe was just finished
    std::vector<item> create_byproducts(int batch = 1) const;

    bool has_byproducts() const;

    bool can_make_with_inventory(const inventory &crafting_inv, int batch = 1) const;
    bool check_eligible_containers_for_crafting(int batch = 1) const;

    int print_items(WINDOW *w, int ypos, int xpos, nc_color col, int batch = 1) const;
    void print_item(WINDOW *w, int ypos, int xpos, nc_color col,
                    const byproduct &bp, int batch = 1) const;
    int print_time(WINDOW *w, int ypos, int xpos, int width, nc_color col,
                   int batch = 1) const;

    int batch_time(int batch = 1) const;

};

/**
*   enum used by comp_selection to indicate where a component should be consumed from.
*/
enum usage {
    use_from_map = 1,
    use_from_player = 2,
    use_from_both = 1 | 2,
    use_from_none = 4,
    cancel = 8 // FIXME: hacky.
};

/**
*   Struct that represents a selection of a component for crafting.
*/
template<typename CompType = component>
struct comp_selection {
    /** Tells us where the selected component should be used from. */
    usage use_from = use_from_none;
    CompType comp;

    /** provides a translated name for 'comp', suffixed with it's location e.g '(nearby)'. */
    std::string nname() const;
};

/**
*   Class that describes a crafting job.
*
*   The class has functions to execute the crafting job.
*/
class craft_command {
    public:
        /** Instantiates an empty craft_command, which can't be executed. */
        craft_command() {}
        craft_command( const recipe *to_make, int batch_size, bool is_long, player *crafter ) :
            rec( to_make ), batch_size( batch_size ), is_long( is_long ), crafter( crafter ) {}

        /** Selects components to use for the craft, then assigns the crafting activity to 'crafter'. */
        void execute();
        /** Consumes the selected components. Must be called after execute(). */
        std::list<item> consume_components();

        bool has_cached_selections() const
        {
            return !item_selections.empty() || !tool_selections.empty();
        }

        bool empty() const
        {
            return rec == nullptr;
        }
    private:
        const recipe *rec = nullptr;
        int batch_size = 0;
        /** Indicates the activity_type for this crafting job, Either ACT_CRAFT or ACT_LONGCRAFT. */
        bool is_long = false;
        player *crafter; // This is mainly here for maintainability reasons.

        std::vector<comp_selection<item_comp>> item_selections;
        std::vector<comp_selection<tool_comp>> tool_selections;

        /** Checks if tools we selected in a previous call to execute() are still available. */
        std::vector<comp_selection<item_comp>>
            check_item_components_missing( const inventory &map_inv ) const;
        /** Checks if items we selected in a previous call to execute() are still available. */
        std::vector<comp_selection<tool_comp>>
            check_tool_components_missing( const inventory &map_inv ) const;

        /** Does a string join with ', ' of the components in the passed vector and inserts into 'str' */
        template<typename T = component>
        void component_list_string( std::stringstream &str,
                                    const std::vector<comp_selection<T>> &components );

        /** Selects components to use */
        void select_components( inventory & map_inv );

        /** Creates a continue pop up asking to continue crafting and listing the missing components */
        bool query_continue( const std::vector<comp_selection<item_comp>> &missing_items,
                             const std::vector<comp_selection<tool_comp>> &missing_tools );
};

/**
*   Repository class for recipes.
*
*   This class is aimed at making (fast) recipe lookups easier from the outside.
*/
class recipe_dictionary {
    public:
        void add( recipe *rec );
        void remove( recipe *rec );
        void clear();

        /** Returns a list of recipes in the 'cat' category */
        const std::vector<recipe *>& in_category( const std::string &cat );
        /** Returns a list of recipes in which the component with itype_id 'id' can be used */
        const std::vector<recipe *>& of_component( const itype_id &id );

        /** Allows for lookup like: 'recipe_dict[name]'. */
        recipe *operator[]( const std::string &rec_name )
        {
            return by_name[rec_name];
        }

        /** Allows for lookup like: 'recipe_dict[id]'. */
        recipe *operator[]( int rec_id )
        {
            return by_index[rec_id];
        }

        size_t size() const
        {
            return recipes.size();
        }

        /** Allows for iteration over all recipes like: 'for( recipe &r : recipe_dict )'. */
        std::list<recipe *>::const_iterator begin() const
        {
            return recipes.begin();
        }
        std::list<recipe *>::const_iterator end() const
        {
            return recipes.end();
        }

    private:
        std::list<recipe *> recipes;

        std::map<const std::string, std::vector<recipe *>> by_category;
        std::map<const itype_id, std::vector<recipe *>> by_component;

        std::map<const std::string, recipe *> by_name;
        std::map<int, recipe *> by_index;

        /** Maps a component to a list of recipes. So we can look up what we can make with an item */
        void add_to_component_lookup( recipe *r );
        void remove_from_component_lookup( recipe *r );
};

extern recipe_dictionary recipe_dict;

// removes any (removable) ammo from the item and stores it in the
// players inventory.
void remove_ammo(item *dis_item, player &p);
// same as above but for each item in the list
void remove_ammo(std::list<item> &dis_items, player &p);

void load_recipe_category(JsonObject &jsobj);
void reset_recipe_categories();
void load_recipe(JsonObject &jsobj);
void reset_recipes();
const recipe *recipe_by_index(int index);
const recipe *recipe_by_name(const std::string &name);
const recipe *get_disassemble_recipe(const itype_id &type);
void finalize_recipes();
// Show the "really disassemble?" query along with a list of possible results.
// Returns false if the player answered no to the query.
bool query_dissamble(const item &dis_item);
const recipe *select_crafting_recipe(int &batch_size);
void pick_recipes(const inventory &crafting_inv,
                  std::vector<const recipe *> &current,
                  std::vector<bool> &available, std::string tab,
                  std::string subtab, std::string filter);
void batch_recipes(const inventory &crafting_inv,
                   std::vector<const recipe *> &current,
                   std::vector<bool> &available, const recipe* r);

void check_recipe_definitions();

void set_item_spoilage(item &newit, float used_age_tally, int used_age_count);
void set_item_food(item &newit);
void set_item_inventory(item &newit);
void finalize_crafted_item(item &newit, float used_age_tally, int used_age_count);

#endif
