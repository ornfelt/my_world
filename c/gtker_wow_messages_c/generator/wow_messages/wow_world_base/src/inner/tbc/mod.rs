pub use crate::manual::tbc::*;
#[cfg(feature = "extended")]
pub use crate::extended::tbc::*;
pub use crate::manual::shared::datetime_vanilla_tbc_wrath::*;
pub use crate::manual::shared::gold_vanilla_tbc_wrath::*;
pub use crate::manual::shared::guid_vanilla_tbc_wrath::*;
pub use crate::manual::shared::level_vanilla_tbc_wrath::*;
pub use crate::manual::shared::player_gender_vanilla_tbc_wrath::*;
pub use crate::manual::shared::player_race_tbc_wrath::*;

pub use crate::shared::account_data_type_vanilla_tbc::*;
pub use crate::shared::activate_taxi_reply_vanilla_tbc_wrath::*;
pub use crate::shared::ai_reaction_vanilla_tbc_wrath::*;
pub(crate) mod area;
pub use area::*;
pub use crate::shared::arena_faction_tbc_wrath::*;
pub use crate::shared::arena_team_command_tbc_wrath::*;
pub use crate::shared::arena_team_command_error_tbc_wrath::*;
pub use crate::shared::arena_team_event_tbc_wrath::*;
pub use crate::shared::arena_team_role_tbc_wrath::*;
pub use crate::shared::arena_type_tbc_wrath::*;
pub use crate::shared::auction_command_action_vanilla_tbc_wrath::*;
pub use crate::shared::auction_command_result_vanilla_tbc_wrath::*;
pub use crate::shared::auction_command_result_two_vanilla_tbc_wrath::*;
pub use crate::shared::auction_house_vanilla_tbc_wrath::*;
pub(crate) mod aura_mod;
pub use aura_mod::*;
pub(crate) mod aura_type;
pub use aura_type::*;
pub use crate::shared::bank_swap_source_tbc_wrath::*;
pub use crate::shared::bank_swap_store_mode_tbc_wrath::*;
pub use crate::shared::battlefield_port_action_vanilla_tbc_wrath::*;
pub(crate) mod battleground_type;
pub use battleground_type::*;
pub(crate) mod bg_type_id;
pub use bg_type_id::*;
pub use crate::shared::bonding_vanilla_tbc_wrath::*;
pub use crate::shared::buy_bank_slot_result_vanilla_tbc_wrath::*;
pub use crate::shared::buy_result_vanilla_tbc_wrath::*;
pub use crate::shared::buyback_slot_vanilla_tbc_wrath::*;
pub use crate::shared::charter_type_tbc_wrath::*;
pub use crate::shared::chat_notify_tbc_wrath::*;
pub(crate) mod chat_restriction_type;
pub use chat_restriction_type::*;
pub(crate) mod chat_type;
pub use chat_type::*;
pub(crate) mod cinematic_sequence_id;
pub use cinematic_sequence_id::*;
pub use crate::shared::class_vanilla_tbc::*;
pub use crate::shared::commentator_enable_option_tbc_wrath::*;
pub use crate::shared::complain_result_window_tbc_wrath::*;
pub use crate::shared::complaint_status_tbc_wrath::*;
pub use crate::shared::corpse_query_result_vanilla_tbc_wrath::*;
pub(crate) mod creature_family;
pub use creature_family::*;
pub use crate::shared::declined_names_tbc_wrath::*;
pub use crate::shared::declined_pet_name_included_tbc_wrath::*;
pub use crate::shared::dispel_method_tbc_wrath::*;
pub use crate::shared::duel_winner_reason_vanilla_tbc_wrath::*;
pub use crate::shared::dungeon_difficulty_tbc_wrath::*;
pub(crate) mod emote;
pub use emote::*;
pub use crate::shared::environmental_damage_type_vanilla_tbc_wrath::*;
pub(crate) mod expansion;
pub use expansion::*;
pub use crate::shared::experience_award_type_vanilla_tbc_wrath::*;
pub(crate) mod faction;
pub use faction::*;
pub use crate::shared::far_sight_operation_vanilla_tbc_wrath::*;
pub use crate::shared::friend_result_vanilla_tbc::*;
pub use crate::shared::friend_status_tbc_wrath::*;
pub use crate::shared::gender_vanilla_tbc_wrath::*;
pub use crate::shared::gm_ticket_escalation_status_vanilla_tbc_wrath::*;
pub use crate::shared::gm_ticket_queue_status_vanilla_tbc_wrath::*;
pub use crate::shared::gm_ticket_response_vanilla_tbc_wrath::*;
pub use crate::shared::gm_ticket_status_vanilla_tbc_wrath::*;
pub use crate::shared::gm_ticket_status_response_vanilla_tbc_wrath::*;
pub(crate) mod gm_ticket_type;
pub use gm_ticket_type::*;
pub use crate::shared::group_loot_setting_vanilla_tbc_wrath::*;
pub use crate::shared::group_type_vanilla_tbc::*;
pub use crate::shared::guild_bank_tab_result_tbc_wrath::*;
pub use crate::shared::guild_command_vanilla_tbc::*;
pub(crate) mod guild_command_result;
pub use guild_command_result::*;
pub use crate::shared::guild_emblem_result_tbc_wrath::*;
pub use crate::shared::guild_event_tbc_wrath::*;
pub use crate::shared::guild_member_status_vanilla_tbc_wrath::*;
pub use crate::shared::hit_info_vanilla_tbc::*;
pub use crate::shared::instance_reset_failed_reason_vanilla_tbc_wrath::*;
pub(crate) mod inventory_result;
pub use inventory_result::*;
pub use crate::shared::inventory_type_vanilla_tbc_wrath::*;
pub use crate::shared::item_class_vanilla_tbc::*;
pub(crate) mod item_class_and_sub_class;
pub use item_class_and_sub_class::*;
pub use crate::shared::item_quality_vanilla_tbc::*;
pub(crate) mod item_set;
pub use item_set::*;
pub(crate) mod item_slot;
pub use item_slot::*;
pub use crate::shared::join_arena_type_tbc_wrath::*;
pub use crate::shared::language_tbc_wrath::*;
pub(crate) mod lfg_mode;
pub use lfg_mode::*;
pub(crate) mod lfg_type;
pub use lfg_type::*;
pub(crate) mod lfg_update_looking_for_more;
pub use lfg_update_looking_for_more::*;
pub use crate::shared::log_format_vanilla_tbc_wrath::*;
pub use crate::shared::logout_result_vanilla_tbc_wrath::*;
pub use crate::shared::logout_speed_vanilla_tbc_wrath::*;
pub use crate::shared::loot_method_vanilla_tbc_wrath::*;
pub use crate::shared::loot_method_error_vanilla_tbc_wrath::*;
pub use crate::shared::loot_slot_type_vanilla_tbc_wrath::*;
pub use crate::shared::mail_action_vanilla_tbc_wrath::*;
pub use crate::shared::mail_message_type_vanilla_tbc_wrath::*;
pub use crate::shared::mail_result_vanilla_tbc::*;
pub use crate::shared::mail_result_two_vanilla_tbc::*;
pub use crate::shared::mail_type_vanilla_tbc_wrath::*;
pub(crate) mod map;
pub use map::*;
pub use crate::shared::meeting_stone_status_vanilla_tbc::*;
pub use crate::shared::monster_move_type_vanilla_tbc_wrath::*;
pub use crate::shared::mount_result_vanilla_tbc_wrath::*;
pub use crate::shared::new_item_chat_alert_vanilla_tbc_wrath::*;
pub use crate::shared::new_item_creation_type_vanilla_tbc_wrath::*;
pub use crate::shared::new_item_source_vanilla_tbc_wrath::*;
pub(crate) mod object_type;
pub use object_type::*;
pub use crate::shared::page_text_material_tbc_wrath::*;
pub use crate::shared::party_operation_vanilla_tbc_wrath::*;
pub use crate::shared::party_result_tbc_wrath::*;
pub use crate::shared::party_role_tbc_wrath::*;
pub use crate::shared::pet_command_state_vanilla_tbc_wrath::*;
pub use crate::shared::pet_enabled_vanilla_tbc_wrath::*;
pub use crate::shared::pet_feedback_vanilla_tbc_wrath::*;
pub use crate::shared::pet_name_invalid_reason_tbc_wrath::*;
pub use crate::shared::pet_query_disabled_names_tbc_wrath::*;
pub use crate::shared::pet_react_state_vanilla_tbc_wrath::*;
pub use crate::shared::pet_talk_reason_vanilla_tbc_wrath::*;
pub use crate::shared::pet_tame_failure_reason_vanilla_tbc::*;
pub use crate::shared::petition_result_tbc_wrath::*;
pub use crate::shared::player_chat_tag_vanilla_tbc::*;
pub use crate::shared::power_vanilla_tbc::*;
pub use crate::shared::pvp_rank_vanilla_tbc_wrath::*;
pub use crate::shared::quest_completable_vanilla_tbc_wrath::*;
pub(crate) mod quest_failed_reason;
pub use quest_failed_reason::*;
pub(crate) mod quest_giver_status;
pub use quest_giver_status::*;
pub use crate::shared::quest_party_message_vanilla_tbc::*;
pub(crate) mod race;
pub use race::*;
pub use crate::shared::raid_group_error_vanilla_tbc_wrath::*;
pub use crate::shared::raid_instance_message_vanilla_tbc::*;
pub use crate::shared::raid_target_index_vanilla_tbc_wrath::*;
pub use crate::shared::raid_target_update_type_vanilla_tbc_wrath::*;
pub use crate::shared::realm_split_state_tbc_wrath::*;
pub use crate::shared::refer_a_friend_error_tbc_wrath::*;
pub use crate::shared::roll_vote_tbc_wrath::*;
pub use crate::shared::sell_item_result_vanilla_tbc::*;
pub use crate::shared::server_message_type_vanilla_tbc::*;
pub use crate::shared::sheath_state_vanilla_tbc_wrath::*;
pub use crate::shared::sheathe_type_vanilla_tbc_wrath::*;
pub use crate::shared::size_class_vanilla_tbc_wrath::*;
pub(crate) mod skill;
pub use skill::*;
pub use crate::shared::skill_category_vanilla_tbc_wrath::*;
pub use crate::shared::spam_type_tbc_wrath::*;
pub(crate) mod spell_cast_result;
pub use spell_cast_result::*;
pub(crate) mod spell_effect;
pub use spell_effect::*;
pub use crate::shared::spell_miss_info_vanilla_tbc_wrath::*;
pub use crate::shared::spell_school_vanilla_tbc_wrath::*;
pub use crate::shared::spell_steal_action_tbc_wrath::*;
pub use crate::shared::spell_trigger_type_tbc_wrath::*;
pub use crate::shared::stable_result_vanilla_tbc::*;
pub use crate::shared::status_id_vanilla_tbc_wrath::*;
pub(crate) mod talent;
pub use talent::*;
pub(crate) mod text_emote;
pub use text_emote::*;
pub use crate::shared::timer_type_vanilla_tbc_wrath::*;
pub use crate::shared::title_earn_status_tbc_wrath::*;
pub use crate::shared::trade_status_vanilla_tbc_wrath::*;
pub use crate::shared::trainer_spell_state_vanilla_tbc_wrath::*;
pub use crate::shared::training_failure_reason_vanilla_tbc_wrath::*;
pub(crate) mod transfer_abort_reason;
pub use transfer_abort_reason::*;
pub use crate::shared::unit_stand_state_vanilla_tbc_wrath::*;
pub use crate::shared::update_type_vanilla_tbc::*;
pub use crate::shared::weather_change_type_vanilla_tbc_wrath::*;
pub(crate) mod weather_type;
pub use weather_type::*;
pub(crate) mod world_result;
pub use world_result::*;
pub use crate::shared::allowed_class_vanilla_tbc::*;
pub(crate) mod allowed_race;
pub use allowed_race::*;
pub use crate::shared::bag_family_tbc_wrath::*;
pub use crate::shared::billing_plan_flags_tbc_wrath::*;
pub use crate::shared::cast_flags_vanilla_tbc::*;
pub use crate::shared::channel_flags_vanilla_tbc_wrath::*;
pub use crate::shared::channel_member_flags_vanilla_tbc_wrath::*;
pub use crate::shared::faction_flag_vanilla_tbc::*;
pub use crate::shared::group_member_online_status_vanilla_tbc_wrath::*;
pub(crate) mod group_update_flags;
pub use group_update_flags::*;
pub(crate) mod item_flag;
pub use item_flag::*;
pub(crate) mod movement_flags;
pub use movement_flags::*;
pub use crate::shared::relation_type_tbc_wrath::*;
pub(crate) mod spell_cast_target_flags;
pub use spell_cast_target_flags::*;
pub use crate::shared::spline_flag_vanilla_tbc::*;
pub use crate::shared::update_flag_vanilla_tbc::*;
pub(crate) mod aura;
pub use aura::*;
pub use crate::shared::item_damage_type_vanilla_tbc_wrath::*;
pub use crate::shared::item_socket_tbc_wrath::*;
pub use crate::shared::item_spells_tbc_wrath::*;
pub use crate::shared::item_stat_tbc_wrath::*;
pub use crate::shared::vector2d_vanilla_tbc_wrath::*;
pub use crate::shared::vector3d_vanilla_tbc_wrath::*;