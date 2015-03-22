#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "player.h"
#include "output.h"
#include "npc.h"
#include "mission.h"
#include <vector>
#include <string>

struct talk_response;
struct dialogue {
    /**
     * The player character that speaks (always g->u).
     * TODO: make it a reference, not a pointer.
     */
    player *alpha;
    /**
     * The NPC we talk to. Never null.
     * TODO: make it a reference, not a pointer.
     */
    npc *beta;
    WINDOW *win;
    /**
     * If true, we are done talking and the dialog ends.
     */
    bool done;
    /**
     * This contains the exchanged words, it is basically like the global message log.
     * Each responses of the player character and the NPC are added as are information about
     * what each of them does (e.g. the npc drops their weapon).
     * This will be displayed in the dialog window and should already be translated.
     */
    std::vector<std::string> history;
    std::vector<talk_topic> topic_stack;

    /** Missions that have been assigned by this npc to the player they currently speak to. */
    std::vector<mission*> missions_assigned;

    int opt(std::string challenge, ...);
    talk_topic opt(talk_topic topic);

    dialogue()
    {
        alpha = NULL;
        beta = NULL;
        win = NULL;
        done = false;
    }

    std::string dynamic_line( talk_topic topic ) const;
    std::vector<talk_response> gen_responses( talk_topic topic ) const;
};

struct talk_function {
    void nothing              (npc *) {};
    void assign_mission       (npc *);
    void mission_success      (npc *);
    void mission_failure      (npc *);
    void clear_mission        (npc *);
    void mission_reward       (npc *);
    void mission_reward_cash  (npc *);
    void mission_favor        (npc *);
    void give_equipment       (npc *);
    void start_trade          (npc *);
    std::string bulk_trade_inquire   (npc *, itype_id);
    void bulk_trade_accept    (npc *, itype_id);
    void assign_base          (npc *);
    void assign_guard         (npc *);
    void stop_guard           (npc *);
    void end_conversation     (npc *);
    void insult_combat        (npc *);
    void reveal_stats         (npc *);
    void follow               (npc *); // p follows u
    void deny_follow          (npc *); // p gets "asked_to_follow"
    void deny_lead            (npc *); // p gets "asked_to_lead"
    void deny_equipment       (npc *); // p gets "asked_for_item"
    void deny_train           (npc *); // p gets "asked_to_train"
    void deny_personal_info   (npc *); // p gets "asked_personal_info"
    void enslave              (npc *) {}; // p becomes slave of u
    void hostile              (npc *); // p turns hostile to u
    void flee                 (npc *);
    void leave                (npc *); // p becomes indifferant
    void stranger_neutral     (npc *); // p is now neutral towards you

    void start_mugging        (npc *);
    void player_leaving       (npc *);

    void drop_weapon          (npc *);
    void player_weapon_away   (npc *);
    void player_weapon_drop   (npc *);

    void lead_to_safety       (npc *);
    void start_training       (npc *);

    void toggle_use_guns      (npc *);
    void toggle_use_silent    (npc *);
    void toggle_use_grenades  (npc *);
    void set_engagement_none  (npc *);
    void set_engagement_close (npc *);
    void set_engagement_weak  (npc *);
    void set_engagement_hit   (npc *);
    void set_engagement_all   (npc *);
};

enum talk_trial {
    TALK_TRIAL_NONE, // No challenge here!
    TALK_TRIAL_LIE, // Straight up lying
    TALK_TRIAL_PERSUADE, // Convince them
    TALK_TRIAL_INTIMIDATE, // Physical intimidation
    NUM_TALK_TRIALS
};

/**
 * This defines possible responses from the player character.
 */
struct talk_response {
    /**
     * What the player character says (literally). Should already be translated and will be
     * displayed. The first character controls the color of it ('*'/'&'/'!').
     */
    std::string text;
    /**
     * If not TALK_TRIAL_NONE, it defines how to decide whether the responses succeeds (e.g. the
     * NPC believes the lie). The difficulty is a 0...100 percent chance of success (!), 100 means
     * always success, 0 means never. It is however affected by mutations/traits/bionics/etc. of
     * the player character. See @ref trial_chance.
     */
    talk_trial trial;
    int difficulty;
    /**
     * The following values are forwarded to the chatbin of the NPC (see @ref npc_chatbin).
     * Except @ref miss, it is apparently not used but should be a mission type that can create
     * new mission.
     */
    mission *mission_selected;
    mission_type_id miss; // If it generates a new mission
    int tempvalue; // Used for various stuff
    const Skill* skill;
    matype_id style;
    /**
     * The following defines what happens when the trial succeeds or fails. If trial is
     * TALK_TRIAL_NONE it always succeeds.
     * talk_topic is the topic that will be handled next, opinion is added to the NPC's opinion
     * of the player character (@ref npc::op_of_u) and the effect function is called.
     */
    npc_opinion opinion_success;
    npc_opinion opinion_failure;
    void (talk_function::*effect_success)(npc *);
    void (talk_function::*effect_failure)(npc *);
    talk_topic success;
    talk_topic failure;

    talk_response()
    {
        text = "";
        trial = TALK_TRIAL_NONE;
        difficulty = 0;
        mission_selected = nullptr;
        miss = MISSION_NULL;
        tempvalue = -1;
        skill = NULL;
        style = "";
        effect_success = &talk_function::nothing;
        effect_failure = &talk_function::nothing;
        opinion_success = npc_opinion();
        opinion_failure = npc_opinion();
        success = TALK_NONE;
        failure = TALK_NONE;
    }
};

struct talk_response_list {
    std::vector<talk_response> none(npc *);
    std::vector<talk_response> shelter(npc *);
    std::vector<talk_response> shopkeep(npc *);
};

/* There is a array of tag_data, "tags", at the bottom of this file.
 * It maps tags to the array of string replacements;
 * e.g. "<name_g>" => talk_good_names
 * Other tags, like "<yrwp>", are mapped to dynamic things
 *  (like the player's weapon), and are defined in parse_tags() (npctalk.cpp)
 */
struct tag_data {
    std::string tag;
    std::string (*replacement)[10];
};

#endif
