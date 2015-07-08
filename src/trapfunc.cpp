#include "game.h"
#include "map.h"
#include "debug.h"
#include "trap.h"
#include "rng.h"
#include "monstergenerator.h"
#include "messages.h"
#include "sounds.h"
#include "translations.h"
#include "event.h"
#include "npc.h"
#include "monster.h"
#include "mapdata.h"
#include "mtype.h"

// A pit becomes less effective as it fills with corpses.
float pit_effectiveness( const tripoint &p )
{
    int corpse_volume = 0;
    for( auto &pit_content : g->m.i_at( p ) ) {
        if( pit_content.is_corpse() ) {
            corpse_volume += pit_content.volume();
        }
    }

    int filled_volume = 75 * 10; // 10 zombies; see item::volume

    return std::max( 0.0f, 1.0f - float( corpse_volume ) / filled_volume );
}

void trapfunc::bubble( Creature *c, const tripoint &p )
{
    // tiny animals don't trigger bubble wrap
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_warning, _( "You step on some bubble wrap!" ),
                                  _( "<npcname> steps on some bubble wrap!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Stepped on bubble wrap." ),
                             pgettext( "memorial_female", "Stepped on bubble wrap." ) );
    }
    sounds::sound( p, 18, _( "Pop!" ) );
    g->m.remove_trap( p );
}

void trapfunc::cot( Creature *c, const tripoint& )
{
    monster *z = dynamic_cast<monster *>( c );
    if( z != nullptr ) {
        // Haha, only monsters stumble over a cot, humans are smart.
        add_msg( m_good, _( "The %s stumbles over the cot" ), z->name().c_str() );
        c->moves -= 100;
    }
}

void trapfunc::beartrap( Creature *c, const tripoint &p )
{
    // tiny animals don't trigger bear traps
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    sounds::sound( p, 8, _( "SNAP!" ) );
    g->m.remove_trap( p );
    if( c != nullptr ) {
        // What got hit?
        body_part hit = num_bp;
        if( one_in( 2 ) ) {
            hit = bp_leg_l;
        } else {
            hit = bp_leg_r;
        }

        // Messages
        c->add_memorial_log( pgettext( "memorial_male", "Caught by a beartrap." ),
                             pgettext( "memorial_female", "Caught by a beartrap." ) );
        c->add_msg_player_or_npc( m_bad, _( "A bear trap closes on your foot!" ),
                                  _( "A bear trap closes on <npcname>'s foot!" ) );

        // Actual effects
        c->add_effect( "beartrap", 1, hit, true );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( z != nullptr ) {
            z->apply_damage( nullptr, hit, 30 );
        } else if( n != nullptr ) {
            damage_instance d;
            d.add_damage( DT_BASH, 12 );
            d.add_damage( DT_CUT, 18 );
            n->deal_damage( nullptr, hit, d );

            if( ( n->has_trait( "INFRESIST" ) ) && ( one_in( 512 ) ) ) {
                n->add_effect( "tetanus", 1, num_bp, true );
            } else if( ( !n->has_trait( "INFIMMUNE" ) || !n->has_trait( "INFRESIST" ) ) && ( one_in( 128 ) ) ) {
                n->add_effect( "tetanus", 1, num_bp, true );
            }
        }
        c->check_dead_state();
    } else {
        g->m.spawn_item( p, "beartrap" );
    }
}

void trapfunc::board( Creature *c, const tripoint& )
{
    // tiny animals don't trigger spiked boards, they can squeeze between the nails
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_memorial_log( pgettext( "memorial_male", "Stepped on a spiked board." ),
                             pgettext( "memorial_female", "Stepped on a spiked board." ) );
        c->add_msg_player_or_npc( m_bad, _( "You step on a spiked board!" ),
                                  _( "<npcname> steps on a spiked board!" ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( z != nullptr ) {
            z->moves -= 80;
            z->apply_damage( nullptr, bp_foot_l, rng( 3, 5 ) );
            z->apply_damage( nullptr, bp_foot_r, rng( 3, 5 ) );
        } else {
            c->deal_damage( nullptr, bp_foot_l, damage_instance( DT_CUT, rng( 6, 10 ) ) );
            c->deal_damage( nullptr, bp_foot_r, damage_instance( DT_CUT, rng( 6, 10 ) ) );
            if( ( n->has_trait( "INFRESIST" ) ) && ( one_in( 256 ) ) ) {
                n->add_effect( "tetanus", 1, num_bp, true );
            } else if( ( !n->has_trait( "INFIMMUNE" ) || !n->has_trait( "INFRESIST" ) ) && ( one_in( 35 ) ) ) {
                n->add_effect( "tetanus", 1, num_bp, true );
            }
        }
        c->check_dead_state();
    }
}

void trapfunc::caltrops( Creature *c, const tripoint& )
{
    // tiny animals don't trigger caltrops, they can squeeze between them
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_memorial_log( pgettext( "memorial_male", "Stepped on a caltrop." ),
                             pgettext( "memorial_female", "Stepped on a caltrop." ) );
        c->add_msg_player_or_npc( m_bad, _( "You step on a sharp metal caltrop!" ),
                                  _( "<npcname> steps on a sharp metal caltrop!" ) );
        monster *z = dynamic_cast<monster *>( c );
        if( z != nullptr ) {
            z->moves -= 80;
            c->apply_damage( nullptr, bp_foot_l, rng( 9, 15 ) );
            c->apply_damage( nullptr, bp_foot_r, rng( 9, 15 ) );
        } else {
            c->deal_damage( nullptr, bp_foot_l, damage_instance( DT_CUT, rng( 9, 30 ) ) );
            c->deal_damage( nullptr, bp_foot_r, damage_instance( DT_CUT, rng( 9, 30 ) ) );
        }
        c->check_dead_state();
    }
}

void trapfunc::tripwire( Creature *c, const tripoint &p )
{
    // tiny animals don't trigger tripwires, they just squeeze under it
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_memorial_log( pgettext( "memorial_male", "Tripped on a tripwire." ),
                             pgettext( "memorial_female", "Tripped on a tripwire." ) );
        c->add_msg_player_or_npc( m_bad, _( "You trip over a tripwire!" ),
                                  _( "<npcname> trips over a tripwire!" ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( z != nullptr ) {
            z->stumble( false );
            if( rng( 0, 10 ) > z->get_dodge() ) {
                z->apply_damage( nullptr, bp_torso, rng( 1, 4 ) );
            }
        } else if( n != nullptr ) {
            std::vector<tripoint> valid;
            tripoint jk = p;
            for( jk.x = p.x - 1; jk.x <= p.x + 1; jk.x++ ) {
                for( jk.y = p.y - 1; jk.y <= p.y + 1; jk.y++ ) {
                    if( g->is_empty( jk ) ) {
                        // No monster, NPC, or player, plus valid for movement
                        valid.push_back( jk );
                    }
                }
            }
            if( !valid.empty() ) {
                jk = valid[rng( 0, valid.size() - 1 )];
                n->setpos( jk );
            }
            n->moves -= 150;
            if( rng( 5, 20 ) > n->dex_cur ) {
                n->hurtall( rng( 1, 4 ), nullptr );
            }
            if( c == &g->u ) {
                g->update_map( &g->u );
            }
        }
        c->check_dead_state();
    }
}

void trapfunc::crossbow( Creature *c, const tripoint &p )
{
    bool add_bolt = true;
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_neutral, _( "You trigger a crossbow trap!" ),
                                  _( "<npcname> triggers a crossbow trap!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a crossbow trap." ),
                             pgettext( "memorial_female", "Triggered a crossbow trap." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            if( !one_in( 4 ) && rng( 8, 20 ) > n->get_dodge() ) {
                body_part hit = num_bp;
                switch( rng( 1, 10 ) ) {
                    case  1:
                        if( one_in( 2 ) ) {
                            hit = bp_foot_l;
                        } else {
                            hit = bp_foot_r;
                        }
                        break;
                    case  2:
                    case  3:
                    case  4:
                        if( one_in( 2 ) ) {
                            hit = bp_leg_l;
                        } else {
                            hit = bp_leg_r;
                        }
                        break;
                    case  5:
                    case  6:
                    case  7:
                    case  8:
                    case  9:
                        hit = bp_torso;
                        break;
                    case 10:
                        hit = bp_head;
                        break;
                }
                //~ %s is bodypart
                n->add_msg_if_player( m_bad, _( "Your %s is hit!" ), body_part_name( hit ).c_str() );
                c->deal_damage( nullptr, hit, damage_instance( DT_CUT, rng( 20, 30 ) ) );
                add_bolt = !one_in( 10 );
            } else {
                n->add_msg_player_or_npc( m_neutral, _( "You dodge the shot!" ),
                                          _( "<npcname> dodges the shot!" ) );
            }
        } else if( z != nullptr ) {
            bool seen = g->u.sees( *z );
            int chance = 0;
            // adapted from shotgun code - chance of getting hit depends on size
            switch( z->type->size ) {
                case MS_TINY:
                    chance = 50;
                    break;
                case MS_SMALL:
                    chance =  8;
                    break;
                case MS_MEDIUM:
                    chance =  6;
                    break;
                case MS_LARGE:
                    chance =  4;
                    break;
                case MS_HUGE:
                    chance =  1;
                    break;
            }
            if( one_in( chance ) ) {
                if( seen ) {
                    add_msg( m_bad, _( "A bolt shoots out and hits the %s!" ), z->name().c_str() );
                }
                z->apply_damage( nullptr, bp_torso, rng( 20, 30 ) );
                add_bolt = !one_in( 10 );
            } else if( seen ) {
                add_msg( m_neutral, _( "A bolt shoots out, but misses the %s." ), z->name().c_str() );
            }
        }
        c->check_dead_state();
    }
    g->m.remove_trap( p );
    g->m.spawn_item( p, "crossbow" );
    g->m.spawn_item( p, "string_6" );
    if( add_bolt ) {
        g->m.spawn_item( p, "bolt_steel", 1, 1 );
    }
}

void trapfunc::shotgun( Creature *c, const tripoint &p )
{
    sounds::sound( p, 60, _( "Kerblam!" ) );
    int shots = 1;
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_neutral, _( "You trigger a shotgun trap!" ),
                                  _( "<npcname> triggers a shotgun trap!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a shotgun trap." ),
                             pgettext( "memorial_female", "Triggered a shotgun trap." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            shots = ( one_in( 8 ) || one_in( 20 - n->str_max ) ? 2 : 1 );
            if( g->m.tr_at( p ).loadid == tr_shotgun_1 ) {
                shots = 1;
            }
            if( rng( 5, 50 ) > n->get_dodge() ) {
                body_part hit = num_bp;
                switch( rng( 1, 10 ) ) {
                    case  1:
                        if( one_in( 2 ) ) {
                            hit = bp_foot_l;
                        } else {
                            hit = bp_foot_r;
                        }
                        break;
                    case  2:
                    case  3:
                    case  4:
                        if( one_in( 2 ) ) {
                            hit = bp_leg_l;
                        } else {
                            hit = bp_leg_r;
                        }
                        break;
                    case  5:
                    case  6:
                    case  7:
                    case  8:
                    case  9:
                        hit = bp_torso;
                        break;
                    case 10:
                        hit = bp_head;
                        break;
                }
                //~ %s is bodypart
                n->add_msg_if_player( m_bad, _( "Your %s is hit!" ), body_part_name( hit ).c_str() );
                c->deal_damage( nullptr, hit, damage_instance( DT_CUT, rng( 40 * shots, 60 * shots ) ) );
            } else {
                n->add_msg_player_or_npc( m_neutral, _( "You dodge the shot!" ),
                                          _( "<npcname> dodges the shot!" ) );
            }
        } else if( z != nullptr ) {
            bool seen = g->u.sees( *z );
            int chance = 0;
            switch( z->type->size ) {
                case MS_TINY:
                    chance = 100;
                    break;
                case MS_SMALL:
                    chance =  16;
                    break;
                case MS_MEDIUM:
                    chance =  12;
                    break;
                case MS_LARGE:
                    chance =   8;
                    break;
                case MS_HUGE:
                    chance =   2;
                    break;
            }
            shots = ( one_in( 8 ) || one_in( chance ) ? 2 : 1 );
            if( g->m.tr_at( p ).loadid == tr_shotgun_1 ) {
                shots = 1;
            }
            if( seen ) {
                add_msg( m_bad, _( "A shotgun fires and hits the %s!" ), z->name().c_str() );
            }
            z->apply_damage( nullptr, bp_torso, rng( 40 * shots, 60 * shots ) );
        }
        c->check_dead_state();
    }
    if( shots == 2 || g->m.tr_at( p ).loadid == tr_shotgun_1 ) {
        g->m.remove_trap( p );
        g->m.spawn_item( p, "shotgun_sawn" );
        g->m.spawn_item( p, "string_6" );
    } else {
        g->m.add_trap( p, tr_shotgun_1 );
    }
}


void trapfunc::blade( Creature *c, const tripoint& )
{
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "A blade swings out and hacks your torso!" ),
                                  _( "A blade swings out and hacks <npcname>s torso!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a blade trap." ),
                             pgettext( "memorial_female", "Triggered a blade trap." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            damage_instance d;
            d.add_damage( DT_BASH, 12 );
            d.add_damage( DT_CUT, 30 );
            n->deal_damage( nullptr, bp_torso, d );
        } else if( z != nullptr ) {
            int cutdam = std::max( 0, 30 - z->get_armor_cut( bp_torso ) );
            int bashdam = std::max( 0, 12 - z->get_armor_bash( bp_torso ) );
            // TODO: move the armor stuff above into monster::deal_damage_handle_type and call
            // Creature::hit for player *and* monster
            z->apply_damage( nullptr, bp_torso, bashdam + cutdam );
        }
        c->check_dead_state();
    }
}

void trapfunc::snare_light( Creature *c, const tripoint &p )
{
    sounds::sound( p, 2, _( "Snap!" ) );
    g->m.remove_trap( p );
    if( c != nullptr ) {
        // Determine what gets hit
        body_part hit = num_bp;
        if( one_in( 2 ) ) {
            hit = bp_leg_l;
        } else {
            hit = bp_leg_r;
        }
        // Messages
        c->add_msg_player_or_npc( m_bad, _( "A snare closes on your leg." ),
                                  _( "A snare closes on <npcname>s leg." ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a light snare." ),
                             pgettext( "memorial_female", "Triggered a light snare." ) );

        // Actual effects
        c->add_effect( "lightsnare", 1, hit, true );
        monster *z = dynamic_cast<monster *>( c );
        if( z != nullptr && z->type->size == MS_TINY ) {
            z->apply_damage( nullptr, one_in( 2 ) ? bp_leg_l : bp_leg_r, 10 );
        }
        c->check_dead_state();
    }
}

void trapfunc::snare_heavy( Creature *c, const tripoint &p )
{
    sounds::sound( p, 4, _( "Snap!" ) );
    g->m.remove_trap( p );
    if( c != nullptr ) {
        // Determine waht got hit
        body_part hit = num_bp;
        if( one_in( 2 ) ) {
            hit = bp_leg_l;
        } else {
            hit = bp_leg_r;
        }
        //~ %s is bodypart name in accusative.
        c->add_msg_player_or_npc( m_bad, _( "A snare closes on your %s." ),
                                  _( "A snare closes on <npcname>s %s." ), body_part_name_accusative( hit ).c_str() );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a heavy snare." ),
                             pgettext( "memorial_female", "Triggered a heavy snare." ) );

        // Actual effects
        c->add_effect( "heavysnare", 1, hit, true );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            damage_instance d;
            d.add_damage( DT_BASH, 10 );
            n->deal_damage( nullptr, hit, d );
        } else if( z != nullptr ) {
            int damage;
            switch( z->type->size ) {
                case MS_TINY:
                    damage = 20;
                    break;
                case MS_SMALL:
                    damage = 20;
                    break;
                case MS_MEDIUM:
                    damage = 10;
                    break;
                default:
                    damage = 0;
            }
            z->apply_damage( nullptr, hit, damage );
        }
        c->check_dead_state();
    }
}

void trapfunc::landmine( Creature *c, const tripoint &p )
{
    // tiny animals are too light to trigger land mines
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "You trigger a land mine!" ),
                                  _( "<npcname> triggers a land mine!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Stepped on a land mine." ),
                             pgettext( "memorial_female", "Stepped on a land mine." ) );
    }
    g->explosion( p, 10, 8, false );
    g->m.remove_trap( p );
}

void trapfunc::boobytrap( Creature *c, const tripoint &p )
{
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "You trigger a booby trap!" ),
                                  _( "<npcname> triggers a booby trap!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a booby trap." ),
                             pgettext( "memorial_female", "Triggered a booby trap." ) );
    }
    g->explosion( p, 18, 12, false );
    g->m.remove_trap( p );
}

void trapfunc::telepad( Creature *c, const tripoint &p )
{
    //~ the sound of a telepad functioning
    sounds::sound( p, 6, _( "vvrrrRRMM*POP!*" ) );
    if( c != nullptr ) {
        monster *z = dynamic_cast<monster *>( c );
        // TODO: NPC don't teleport?
        if( c == &g->u ) {
            c->add_msg_if_player( m_warning, _( "The air shimmers around you..." ) );
            c->add_memorial_log( pgettext( "memorial_male", "Triggered a teleport trap." ),
                                 pgettext( "memorial_female", "Triggered a teleport trap." ) );
            g->teleport();
        } else if( z != nullptr ) {
            if( g->u.sees( *z ) ) {
                add_msg( _( "The air shimmers around the %s..." ), z->name().c_str() );
            }

            int tries = 0;
            int newposx, newposy;
            do {
                newposx = rng( z->posx() - SEEX, z->posx() + SEEX );
                newposy = rng( z->posy() - SEEY, z->posy() + SEEY );
                tries++;
            } while( g->m.move_cost( newposx, newposy ) == 0 && tries != 10 );

            if( tries == 10 ) {
                z->die_in_explosion( nullptr );
            } else {
                int mon_hit = g->mon_at( {newposx, newposy, z->posz()} );
                if( mon_hit != -1 ) {
                    if( g->u.sees( *z ) ) {
                        add_msg( m_good, _( "The %s teleports into a %s, killing them both!" ),
                                 z->name().c_str(), g->zombie( mon_hit ).name().c_str() );
                    }
                    g->zombie( mon_hit ).die_in_explosion( z );
                } else {
                    z->setpos( {newposx, newposy, z->posz()} );
                }
            }
        }
    }
}

void trapfunc::goo( Creature *c, const tripoint &p )
{
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "You step in a puddle of thick goo." ),
                                  _( "<npcname> steps in a puddle of thick goo." ) );
        c->add_memorial_log( pgettext( "memorial_male", "Stepped into thick goo." ),
                             pgettext( "memorial_female", "Stepped into thick goo." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            n->add_env_effect( "slimed", bp_foot_l, 6, 20 );
            n->add_env_effect( "slimed", bp_foot_r, 6, 20 );
            if( one_in( 3 ) ) {
                n->add_msg_if_player( m_bad, _( "The acidic goo eats away at your feet." ) );
                n->deal_damage( nullptr, bp_foot_l, damage_instance( DT_CUT, 5 ) );
                n->deal_damage( nullptr, bp_foot_r, damage_instance( DT_CUT, 5 ) );
                n->check_dead_state();
            }
        } else if( z != nullptr ) {
            if( z->type->id == "mon_blob" ) {
                z->set_speed_base( z->get_speed_base() + 15 );
                z->set_hp( z->get_speed() );
            } else {
                z->poly( GetMType( "mon_blob" ) );
                z->set_speed_base( z->get_speed_base() - 15 );
                z->set_hp( z->get_speed() );
            }
        }
    }
    g->m.remove_trap( p );
}

void trapfunc::dissector( Creature *c, const tripoint &p )
{
    //~ the sound of a dissector dissecting
    sounds::sound( p, 10, _( "BRZZZAP!" ) );
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "Electrical beams emit from the floor and slice your flesh!" ),
                                  _( "Electrical beams emit from the floor and slice <npcname>s flesh!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Stepped into a dissector." ),
                             pgettext( "memorial_female", "Stepped into a dissector." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            n->deal_damage( nullptr, bp_head, damage_instance( DT_CUT, 15 ) );
            n->deal_damage( nullptr, bp_torso, damage_instance( DT_CUT, 20 ) );
            n->deal_damage( nullptr, bp_arm_r, damage_instance( DT_CUT, 12 ) );
            n->deal_damage( nullptr, bp_arm_l, damage_instance( DT_CUT, 12 ) );
            n->deal_damage( nullptr, bp_hand_r, damage_instance( DT_CUT, 10 ) );
            n->deal_damage( nullptr, bp_hand_l, damage_instance( DT_CUT, 10 ) );
            n->deal_damage( nullptr, bp_leg_r, damage_instance( DT_CUT, 12 ) );
            n->deal_damage( nullptr, bp_leg_r, damage_instance( DT_CUT, 12 ) );
            n->deal_damage( nullptr, bp_foot_l, damage_instance( DT_CUT, 10 ) );
            n->deal_damage( nullptr, bp_foot_r, damage_instance( DT_CUT, 10 ) );
        } else if( z != nullptr ) {
            z->apply_damage( nullptr, bp_torso, 60 );
            if( z->is_dead() ) {
                z->explode();
            }
        }
        c->check_dead_state();
    }
}

void trapfunc::pit( Creature *c, const tripoint &p )
{
    // tiny animals aren't hurt by falling into pits
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        const float eff = pit_effectiveness( p );
        c->add_msg_player_or_npc( m_bad, _( "You fall in a pit!" ), _( "<npcname> falls in a pit!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Fell in a pit." ),
                             pgettext( "memorial_female", "Fell in a pit." ) );
        c->add_effect( "in_pit", 1, num_bp, true );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            if( ( n->has_trait( "WINGS_BIRD" ) ) || ( ( one_in( 2 ) ) &&
                    ( n->has_trait( "WINGS_BUTTERFLY" ) ) ) ) {
                n->add_msg_if_player( _( "You flap your wings and flutter down gracefully." ) );
            } else if (n->has_trait("WINGS_DRAGON") ) {
                n->add_msg_if_player(_("You spread your wings and glide down safely."));
            } else {
                int dodge = n->get_dodge();
                int damage = eff * rng( 10, 20 ) - rng( dodge, dodge * 5 );
                if( damage > 0 ) {
                    n->add_msg_if_player( m_bad, _( "You hurt yourself!" ) );
                    n->hurtall( rng( int( damage / 2 ), damage ), n ); // like the message says \-:
                    n->deal_damage( nullptr, bp_leg_l, damage_instance( DT_BASH, damage ) );
                    n->deal_damage( nullptr, bp_leg_r, damage_instance( DT_BASH, damage ) );
                } else {
                    n->add_msg_if_player( _( "You land nimbly." ) );
                }
            }
        } else if( z != nullptr ) {
            z->apply_damage( nullptr, bp_torso, eff * rng( 10, 20 ) );
        }
        c->check_dead_state();
    }
}

void trapfunc::pit_spikes( Creature *c, const tripoint &p )
{
    // tiny animals aren't hurt by falling into spiked pits
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "You fall in a spiked pit!" ),
                                  _( "<npcname> falls in a spiked pit!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Fell into a spiked pit." ),
                             pgettext( "memorial_female", "Fell into a spiked pit." ) );
        c->add_effect( "in_pit", 1, num_bp, true );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            int dodge = n->get_dodge();
            int damage = pit_effectiveness( p ) * rng( 20, 50 );
            if( ( n->has_trait( "WINGS_BIRD" ) ) || ( ( one_in( 2 ) ) &&
                    ( n->has_trait( "WINGS_BUTTERFLY" ) ) ) ) {
                n->add_msg_if_player( _( "You flap your wings and flutter down gracefully." ) );
            } else if (n->has_trait("WINGS_DRAGON") ) {
                n->add_msg_if_player(_("You spread your wings and glide down safely."));
            } else if( 0 == damage || rng( 5, 30 ) < dodge ) {
                n->add_msg_if_player( _( "You avoid the spikes within." ) );
            } else {
                body_part hit = num_bp;
                switch( rng( 1, 10 ) ) {
                    case  1:
                        hit = bp_leg_l;
                        break;
                    case  2:
                        hit = bp_leg_r;
                        break;
                    case  3:
                        hit = bp_arm_l;
                        break;
                    case  4:
                        hit = bp_arm_r;
                        break;
                    case  5:
                    case  6:
                    case  7:
                    case  8:
                    case  9:
                    case 10:
                        hit = bp_torso;
                        break;
                }
                n->add_msg_if_player( m_bad, _( "The spikes impale your %s!" ),
                                      body_part_name_accusative( hit ).c_str() );
                n->deal_damage( nullptr, hit, damage_instance( DT_CUT, damage ) );
                if( ( n->has_trait( "INFRESIST" ) ) && ( one_in( 256 ) ) ) {
                    n->add_effect( "tetanus", 1, num_bp, true );
                } else if( ( !n->has_trait( "INFIMMUNE" ) || !n->has_trait( "INFRESIST" ) ) && ( one_in( 35 ) ) ) {
                    n->add_effect( "tetanus", 1, num_bp, true );
                }
            }
        } else if( z != nullptr ) {
            z->apply_damage( nullptr, bp_torso, rng( 20, 50 ) );
        }
        c->check_dead_state();
    }
    if( one_in( 4 ) ) {
        if( g->u.sees( p ) ) {
            add_msg( _( "The spears break!" ) );
        }
        g->m.ter_set( p, t_pit );
        for( int i = 0; i < 4; i++ ) { // 4 spears to a pit
            if( one_in( 3 ) ) {
                g->m.spawn_item( p, "pointy_stick" );
            }
        }
    }
}

void trapfunc::pit_glass( Creature *c, const tripoint &p )
{
    // tiny animals aren't hurt by falling into glass pits
    if( c != nullptr && c->get_size() == MS_TINY ) {
        return;
    }
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "You fall in a pit filled with glass shards!" ),
                                  _( "<npcname> falls in pit filled with glass shards!" ) );
        c->add_memorial_log( pgettext( "memorial_male", "Fell into a pit filled with glass shards." ),
                             pgettext( "memorial_female", "Fell into a pit filled with glass shards." ) );
        c->add_effect( "in_pit", 1, num_bp, true );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            int dodge = n->get_dodge();
            int damage = pit_effectiveness( p ) * rng( 15, 35 );
            if( ( n->has_trait( "WINGS_BIRD" ) ) || ( ( one_in( 2 ) ) &&
                    ( n->has_trait( "WINGS_BUTTERFLY" ) ) ) ) {
                n->add_msg_if_player( _( "You flap your wings and flutter down gracefully." ) );
            } else if( ( n->has_trait( "WINGS_DRAGON" ) ) ) {
                n->add_msg_if_player( _( "You spread your wings and glide down safely." ) );
            } else if( 0 == damage || rng( 5, 30 ) < dodge ) {
                n->add_msg_if_player( _( "You avoid the glass shards within." ) );
            } else {
                body_part hit = num_bp;
                switch( rng( 1, 10 ) ) {
                    case  1:
                        hit = bp_leg_l;
                        break;
                    case  2:
                        hit = bp_leg_r;
                        break;
                    case  3:
                        hit = bp_arm_l;
                        break;
                    case  4:
                        hit = bp_arm_r;
                        break;
                    case  5:
                        hit = bp_foot_l;
                        break;
                    case  6:
                        hit = bp_foot_r;
                        break;
                    case  7:
                    case  8:
                    case  9:
                    case 10:
                        hit = bp_torso;
                        break;
                }
                n->add_msg_if_player( m_bad, _( "The glass shards slash your %s!" ),
                                      body_part_name_accusative( hit ).c_str() );
                n->deal_damage( nullptr, hit, damage_instance( DT_CUT, damage ) );
                if( ( n->has_trait( "INFRESIST" ) ) && ( one_in( 256 ) ) ) {
                    n->add_effect( "tetanus", 1, num_bp, true );
                } else if( ( !n->has_trait( "INFIMMUNE" ) || !n->has_trait( "INFRESIST" ) ) && ( one_in( 35 ) ) ) {
                    n->add_effect( "tetanus", 1, num_bp, true );
                }
            }
        } else if( z != nullptr ) {
            z->apply_damage( nullptr, bp_torso, rng( 20, 50 ) );
        }
        c->check_dead_state();
    }
    if( one_in( 5 ) ) {
        if( g->u.sees( p ) ) {
            add_msg( _( "The shards shatter!" ) );
        }
        g->m.ter_set( p, t_pit );
        for( int i = 0; i < 20; i++ ) { // 20 shards in a pit.
            if( one_in( 3 ) ) {
                g->m.spawn_item( p, "glass_shard" );
            }
        }
    }
}

void trapfunc::lava( Creature *c, const tripoint &p )
{
    if( c != nullptr ) {
        c->add_msg_player_or_npc( m_bad, _( "The %s burns you horribly!" ), _( "The %s burns <npcname>!" ),
                                  g->m.tername( p ).c_str() );
        c->add_memorial_log( pgettext( "memorial_male", "Stepped into lava." ),
                             pgettext( "memorial_female", "Stepped into lava." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            n->deal_damage( nullptr, bp_foot_l, damage_instance( DT_HEAT, 20 ) );
            n->deal_damage( nullptr, bp_foot_r, damage_instance( DT_HEAT, 20 ) );
            n->deal_damage( nullptr, bp_leg_l, damage_instance( DT_HEAT, 20 ) );
            n->deal_damage( nullptr, bp_leg_r, damage_instance( DT_HEAT, 20 ) );
        } else if( z != nullptr ) {
            // MATERIALS-TODO: use fire resistance
            int dam = 30;
            if( z->made_of( "flesh" ) || z->made_of( "iflesh" ) ) {
                dam = 50;
            }
            if( z->made_of( "veggy" ) ) {
                dam = 80;
            }
            if( z->made_of( "paper" ) || z->made_of( LIQUID ) || z->made_of( "powder" ) ||
                z->made_of( "wood" )  || z->made_of( "cotton" ) || z->made_of( "wool" ) ) {
                dam = 200;
            }
            if( z->made_of( "stone" ) ) {
                dam = 15;
            }
            if( z->made_of( "kevlar" ) || z->made_of( "steel" ) ) {
                dam = 5;
            }
            z->apply_damage( nullptr, bp_torso, dam );
        }
        c->check_dead_state();
    }
}

// STUB
void trapfunc::portal( Creature * /*c*/, const tripoint& )
{
    // TODO: make this do something?
}

// Don't ask NPCs - they always want to do the first thing that comes to their minds
bool query_for_item( const player *pl, const std::string &itemname, const char *que ) {
    return pl->has_amount( itemname, 1 ) && ( !pl->is_player() || query_yn( que ) );
};

void trapfunc::sinkhole( Creature *c, const tripoint &p )
{
    player *pl = dynamic_cast<player*>( c );
    if( pl == nullptr ) {
        // TODO: Handle monsters
        return;
    }

    const auto random_neighbor = []( tripoint center ) {
        center.x += rng( -1, 1 );
        center.y += rng( -1, 1 );
        return center;
    };

    const auto safety_roll = [&]( const std::string &itemname,
                                  const int diff ) {
        const int roll = rng( pl->skillLevel( "throw" ),
                              pl->skillLevel( "throw" ) + pl->str_cur + pl->dex_cur );
        if( roll < diff ) {
            pl->add_msg_if_player( m_bad, _( "You fail to attach it..." ) );
            pl->use_amount( itemname, 1 );
            g->m.spawn_item( random_neighbor( pl->pos() ), itemname );
            return false;
        }

        std::vector<tripoint> safe;
        tripoint tmp = pl->pos();
        int &i = tmp.x;
        int &j = tmp.y;
        for( i = pl->posx() - 1; i <= pl->posx() + 1; i++ ) {
            for( j = pl->posy() - 1; j <= pl->posy() + 1; j++ ) {
                if( g->m.move_cost( tmp ) > 0 && g->m.tr_at( tmp ).loadid != tr_pit ) {
                    safe.push_back( tmp );
                }
            }
        }
        if( safe.empty() ) {
            pl->add_msg_if_player( m_bad, _( "There's nowhere to pull yourself to, and you sink!" ) );
            pl->use_amount( itemname, 1 );
            g->m.spawn_item( random_neighbor( pl->pos() ), itemname );
            return false;
        } else {
            pl->add_msg_player_or_npc( m_good, _( "You pull yourself to safety!" ),
                                               _( "<npcname> steps on a sinkhole, but manages to pull themselves to safety." ) );
            int index = rng( 0, safe.size() - 1 );
            pl->setpos( safe[index] );
            if( pl == &g->u ) {
                g->update_map( &g->u );
            }

            return true;
        }
    };

    pl->add_memorial_log( pgettext( "memorial_male", "Stepped into a sinkhole." ),
                           pgettext( "memorial_female", "Stepped into a sinkhole." ) );
    bool success = false;
    if( query_for_item( pl, "grapnel", _( "You step into a sinkhole!  Throw your grappling hook out to try to catch something?" ) ) ) {
        success = safety_roll( "grapnel", 6 );
    } else if( query_for_item( pl, "bullwhip", _( "You step into a sinkhole!  Throw your whip out to try and snag something?" ) ) ) {
        success = safety_roll( "bullwhip", 8 );
    } else if( query_for_item( pl, "rope_30", _( "You step into a sinkhole!  Throw your rope out to try to catch something?" ) ) ) {
        success = safety_roll( "rope_30", 12 );
    }

    pl->add_msg_player_or_npc( m_warning, _( "The sinkhole collapses!" ),
                                          _( "A sinkhole under <npcname> collapses!" ) );
    g->m.remove_trap( p );
    g->m.ter_set( p, t_pit );
    if( success ) {
        return;
    }

    pl->moves -= 100;
    pl->add_msg_player_or_npc( m_bad, _( "You fall into the sinkhole!" ),
                                      _( "<npcname> falls into a sinkhole!" ) );
    pit( c, p );
}

void trapfunc::ledge( Creature *c, const tripoint &p )
{
    if( c == nullptr ) {
        return;
    }

    monster *m = dynamic_cast<monster*>( c );
    if( m != nullptr && m->has_flag( MF_FLIES ) ) {
        return;
    }

    if( !g->m.has_zlevels() ) {
        if( c == &g->u ) {
            add_msg( m_warning, _( "You fall down a level!" ) );
            g->u.add_memorial_log( pgettext( "memorial_male", "Fell down a ledge." ),
                                   pgettext( "memorial_female", "Fell down a ledge." ) );
            g->vertical_move( -1, true );
            if( g->u.has_trait("WINGS_BIRD") || ( one_in( 2 ) && g->u.has_trait("WINGS_BUTTERFLY") ) ) {
                add_msg( _("You flap your wings and flutter down gracefully.") );
            } else if( g->u.has_trait("WINGS_DRAGON") ) {
                add_msg( _("You spread your wings and glide down safely.") );
            } else {
                g->u.impact( 20, p );
            }
        } else {
            c->add_msg_if_npc( _( "<npcname> falls down a level!" ) );
            c->die( nullptr );
        }

        return;
    }

    int height = 0;
    tripoint where = p;
    while( g->m.has_flag( TFLAG_NO_FLOOR, where ) ) {
        where.z--;
        if( g->critter_at( where ) != nullptr ) {
            where.z++;
            break;
        }

        height++;
    }

    if( height == 0 ) {
        return;
    }

    c->add_msg_if_npc( _( "<npcname> falls down a level!" ) );
    player *pl = dynamic_cast<player*>( c );
    if( pl == nullptr ) {
        c->setpos( where );
        c->impact( height * 10, where );
        return;
    }

    if( pl->is_player() ) {
        add_msg( m_warning, _( "You fall down a level!" ) );
        g->u.add_memorial_log( pgettext( "memorial_male", "Fell down a ledge." ),
                               pgettext( "memorial_female", "Fell down a ledge." ) );
        g->vertical_move( -height, true );
    } else {
        pl->setpos( where );
    }
    if( pl->has_trait("WINGS_BIRD") || ( one_in( 2 ) && pl->has_trait("WINGS_BUTTERFLY") ) ) {
        pl->add_msg_player_or_npc( _("You flap your wings and flutter down gracefully."),
                                   _("<npcname> flaps their wings and flutters down gracefully.") );
    } else if( pl->has_trait("WINGS_DRAGON") ) {
        pl->add_msg_player_or_npc( _("You spread your wings and glide down safely."),
                                   _("<npcname> spreads their wings and glides down safely.") );
    } else {
        pl->impact( height * 10, where );
    }
}

void trapfunc::temple_flood( Creature *c, const tripoint &p )
{
    // Monsters and npcs are completely ignored here, should they?
    if( c == &g->u ) {
        add_msg( m_warning, _( "You step on a loose tile, and water starts to flood the room!" ) );
        g->u.add_memorial_log( pgettext( "memorial_male", "Triggered a flood trap." ),
                               pgettext( "memorial_female", "Triggered a flood trap." ) );
        tripoint tmp = p;
        int &i = tmp.x;
        int &j = tmp.y;
        for( i = 0; i < SEEX * MAPSIZE; i++ ) {
            for( j = 0; j < SEEY * MAPSIZE; j++ ) {
                if( g->m.tr_at( tmp ).loadid == tr_temple_flood ) {
                    g->m.remove_trap( tmp );
                }
            }
        }
        g->add_event( EVENT_TEMPLE_FLOOD, calendar::turn + 3 );
    }
}

void trapfunc::temple_toggle( Creature *c, const tripoint &p )
{
    // Monsters and npcs are completely ignored here, should they?
    if( c == &g->u ) {
        add_msg( _( "You hear the grinding of shifting rock." ) );
        const ter_id type = g->m.ter( p );
        tripoint tmp = p;
        int &i = tmp.x;
        int &j = tmp.y;
        for( i = 0; i < SEEX * MAPSIZE; i++ ) {
            for( j = 0; j < SEEY * MAPSIZE; j++ ) {
                if( type == t_floor_red ) {
                    if( g->m.ter( tmp ) == t_rock_green ) {
                        g->m.ter_set( tmp, t_floor_green );
                    } else if( g->m.ter( tmp ) == t_floor_green ) {
                        g->m.ter_set( tmp, t_rock_green );
                    }
                } else if( type == t_floor_green ) {
                    if( g->m.ter( tmp ) == t_rock_blue ) {
                        g->m.ter_set( tmp, t_floor_blue );
                    } else if( g->m.ter( tmp ) == t_floor_blue ) {
                        g->m.ter_set( tmp, t_rock_blue );
                    }
                } else if( type == t_floor_blue ) {
                    if( g->m.ter( tmp ) == t_rock_red ) {
                        g->m.ter_set( tmp, t_floor_red );
                    } else if( g->m.ter( tmp ) == t_floor_red ) {
                        g->m.ter_set( tmp, t_rock_red );
                    }
                }
            }
        }
    }
}

void trapfunc::glow( Creature *c, const tripoint &p )
{
    if( c != nullptr ) {
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            if( one_in( 3 ) ) {
                n->add_msg_if_player( m_bad, _( "You're bathed in radiation!" ) );
                n->radiation += rng( 10, 30 );
            } else if( one_in( 4 ) ) {
                n->add_msg_if_player( m_bad, _( "A blinding flash strikes you!" ) );
                g->flashbang( p );
            } else {
                c->add_msg_if_player( _( "Small flashes surround you." ) );
            }
        } else if( z != nullptr && one_in( 3 ) ) {
            z->apply_damage( nullptr, bp_torso, rng( 5, 10 ) );
            z->set_speed_base( z->get_speed_base() * 0.9 );
        }
        c->check_dead_state();
    }
}

void trapfunc::hum( Creature * /*c*/, const tripoint &p )
{
    int volume = rng( 1, 200 );
    std::string sfx;
    if( volume <= 10 ) {
        //~ a quiet humming sound
        sfx = _( "hrm" );
    } else if( volume <= 50 ) {
        //~ a humming sound
        sfx = _( "hrmmm" );
    } else if( volume <= 100 ) {
        //~ a loud humming sound
        sfx = _( "HRMMM" );
    } else {
        //~ a very loud humming sound
        sfx = _( "VRMMMMMM" );
    }
    sounds::sound( p, volume, sfx );
}

void trapfunc::shadow( Creature *c, const tripoint &p )
{
    if( c != &g->u ) {
        return;
    }
    // Monsters and npcs are completely ignored here, should they?
    g->u.add_memorial_log( pgettext( "memorial_male", "Triggered a shadow trap." ),
                           pgettext( "memorial_female", "Triggered a shadow trap." ) );
    int tries = 0;
    tripoint monp = p;
    do {
        if( one_in( 2 ) ) {
            monp.x = rng( g->u.posx() - 5, g->u.posx() + 5 );
            monp.y = ( one_in( 2 ) ? g->u.posy() - 5 : g->u.posy() + 5 );
        } else {
            monp.x = ( one_in( 2 ) ? g->u.posx() - 5 : g->u.posx() + 5 );
            monp.y = rng( g->u.posy() - 5, g->u.posy() + 5 );
        }
    } while( tries < 5 && !g->is_empty( monp ) &&
             !g->m.sees( monp, g->u.pos(), 10 ) );

    if( tries < 5 ) {
        if( g->summon_mon( "mon_shadow", monp ) ) {
            add_msg( m_warning, _( "A shadow forms nearby." ) );
            monster *spawned = g->monster_at( monp );
            spawned->reset_special_rng( 0 );
        }
        g->m.remove_trap( p );
    }
}

void trapfunc::drain( Creature *c, const tripoint& )
{
    if( c != nullptr ) {
        c->add_msg_if_player( m_bad, _( "You feel your life force sapping away." ) );
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a life-draining trap." ),
                             pgettext( "memorial_female", "Triggered a life-draining trap." ) );
        monster *z = dynamic_cast<monster *>( c );
        player *n = dynamic_cast<player *>( c );
        if( n != nullptr ) {
            n->hurtall( 1, nullptr );
        } else if( z != nullptr ) {
            z->apply_damage( nullptr, bp_torso, 1 );
        }
        c->check_dead_state();
    }
}

void trapfunc::snake( Creature *c, const tripoint &p )
{
    //~ the sound a snake makes
    sounds::sound( p, 10, _( "ssssssss" ) );
    if( one_in( 6 ) ) {
        g->m.remove_trap( p );
    }
    if( c != nullptr ) {
        c->add_memorial_log( pgettext( "memorial_male", "Triggered a shadow snake trap." ),
                             pgettext( "memorial_female", "Triggered a shadow snake trap." ) );
    }
    if( one_in( 3 ) ) {
        int tries = 0;
        tripoint monp = p;
        // This spawns snakes only when the player can see them, why?
        do {
            if( one_in( 2 ) ) {
                monp.x = rng( g->u.posx() - 5, g->u.posx() + 5 );
                monp.y = ( one_in( 2 ) ? g->u.posy() - 5 : g->u.posy() + 5 );
            } else {
                monp.x = ( one_in( 2 ) ? g->u.posx() - 5 : g->u.posx() + 5 );
                monp.y = rng( g->u.posy() - 5, g->u.posy() + 5 );
            }
        } while( tries < 5 && !g->is_empty( monp ) &&
                 !g->m.sees( monp, g->u.pos(), 10 ) );

        if( tries < 5 ) {
            add_msg( m_warning, _( "A shadowy snake forms nearby." ) );
            g->summon_mon( "mon_shadow_snake", p );
            g->m.remove_trap( p );
        }
    }
}

void trapfunc::chunkblower(Creature *c, const tripoint &p )
{
    if (c != NULL) {
        c->add_msg_player_or_npc(m_bad, _("Your grinded mash of arms and legs, torsos and heads, now hamburger meat."), _("Your grinded mash of arms and legs, torsos and heads, now hamburger meat."),
                                 g->m.tername(p).c_str());
        c->add_memorial_log(pgettext("memorial_male", "reduced to ground beef."),
                            pgettext("memorial_female", "reduced to ground beef."));
        monster *z = dynamic_cast<monster *>(c);
        player *n = dynamic_cast<player *>(c);
        if (n != NULL) {
            n->deal_damage( nullptr, bp_foot_l, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_foot_r, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_leg_l, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_leg_r, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_hand_l, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_hand_r, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_arm_l, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_arm_r, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_head, damage_instance( DT_CUT, 1000 ) );
            n->deal_damage( nullptr, bp_torso, damage_instance( DT_CUT, 1000 ) );
        } else if (z != NULL) {
            z->apply_damage( nullptr, bp_torso, 10000 );
        }
    }
}
/**
 * Takes the name of a trap function and returns a function pointer to it.
 * @param function_name The name of the trapfunc function to find.
 * @return A function pointer to the matched function, or to trapfunc::none if
 *         there is no match.
 */
trap_function trap_function_from_string( std::string function_name )
{
    if( "none" == function_name ) {
        return &trapfunc::none;
    }
    if( "bubble" == function_name ) {
        return &trapfunc::bubble;
    }
    if( "cot" == function_name ) {
        return &trapfunc::cot;
    }
    if( "beartrap" == function_name ) {
        return &trapfunc::beartrap;
    }
    if( "board" == function_name ) {
        return &trapfunc::board;
    }
    if( "caltrops" == function_name ) {
        return &trapfunc::caltrops;
    }
    if( "tripwire" == function_name ) {
        return &trapfunc::tripwire;
    }
    if( "crossbow" == function_name ) {
        return &trapfunc::crossbow;
    }
    if( "shotgun" == function_name ) {
        return &trapfunc::shotgun;
    }
    if( "blade" == function_name ) {
        return &trapfunc::blade;
    }
    if( "snare_light" == function_name ) {
        return &trapfunc::snare_light;
    }
    if( "snare_heavy" == function_name ) {
        return &trapfunc::snare_heavy;
    }
    if( "landmine" == function_name ) {
        return &trapfunc::landmine;
    }
    if( "telepad" == function_name ) {
        return &trapfunc::telepad;
    }
    if( "goo" == function_name ) {
        return &trapfunc::goo;
    }
    if( "dissector" == function_name ) {
        return &trapfunc::dissector;
    }
    if( "sinkhole" == function_name ) {
        return &trapfunc::sinkhole;
    }
    if( "pit" == function_name ) {
        return &trapfunc::pit;
    }
    if( "pit_spikes" == function_name ) {
        return &trapfunc::pit_spikes;
    }
    if( "pit_glass" == function_name ) {
        return &trapfunc::pit_glass;
    }
    if( "lava" == function_name ) {
        return &trapfunc::lava;
    }
    if( "portal" == function_name ) {
        return &trapfunc::portal;
    }
    if( "ledge" == function_name ) {
        return &trapfunc::ledge;
    }
    if( "boobytrap" == function_name ) {
        return &trapfunc::boobytrap;
    }
    if( "temple_flood" == function_name ) {
        return &trapfunc::temple_flood;
    }
    if( "temple_toggle" == function_name ) {
        return &trapfunc::temple_toggle;
    }
    if( "glow" == function_name ) {
        return &trapfunc::glow;
    }
    if( "hum" == function_name ) {
        return &trapfunc::hum;
    }
    if( "shadow" == function_name ) {
        return &trapfunc::shadow;
    }
    if( "drain" == function_name ) {
        return &trapfunc::drain;
    }
    if( "snake" == function_name ) {
        return &trapfunc::snake;
    }
    if("chunkblower" == function_name) {
        return &trapfunc::chunkblower;
    }

    //No match found
    debugmsg( "Could not find a trapfunc function matching '%s'!", function_name.c_str() );
    return &trapfunc::none;
}
