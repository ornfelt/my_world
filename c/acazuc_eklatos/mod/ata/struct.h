#ifndef DEV_ATA_STRUCT_H
#define DEV_ATA_STRUCT_H

#include <std.h>

struct ata_identify_device
{
	struct
	{
		uint16_t reserved1 : 1;
		uint16_t retired3 : 1;
		uint16_t response_incomplete : 1;
		uint16_t retired2 : 3;
		uint16_t fixed_device : 1;
		uint16_t removable_media : 1;
		uint16_t retired1 : 7;
		uint16_t device_type : 1;
	} general_configuration;
	uint16_t num_cylinders;
	uint16_t specific_configuration;
	uint16_t num_heads;
	uint16_t retired1[2];
	uint16_t num_sectors_per_track;
	uint16_t vendor_unique1[3];
	char serial_number[20];
	uint16_t retired2[2];
	uint16_t obsolete1;
	char firmware_revision[8];
	char model_number[40];
	char maximum_block_transfer;
	char vendor_unique2;
	struct
	{
		uint16_t feature_supported : 1;
		uint16_t reserved : 15;
	} trusted_computing;
	struct
	{
		uint8_t current_long_physical_sector_alignment : 2;
		uint8_t reserved_byte49 : 6;
		uint8_t dma_supported : 1;
		uint8_t lba_supported : 1;
		uint8_t Iordy_disable : 1;
		uint8_t Iordy_supported : 1;
		uint8_t reserved1 : 1;
		uint8_t standyby_timer_support : 1;
		uint8_t reserved2 : 2;
		uint16_t reserved_word50;
	} capabilities;
	uint16_t obsolete_words51[2];
	uint16_t translation_fields_valid : 3;
	uint16_t reserved3 : 5;
	uint16_t free_fall_control_sensitivity : 8;
	uint16_t number_of_current_cylinders;
	uint16_t number_of_current_heads;
	uint16_t current_sectors_per_track;
	uint32_t current_sector_capacity;
	uint8_t current_multi_sector_setting;
	uint8_t multi_sector_setting_valid : 1;
	uint8_t reserved_byte59 : 3;
	uint8_t sanitize_feature_supported : 1;
	uint8_t crypto_scramble_ext_command_supported : 1;
	uint8_t overwrite_ext_command_supported : 1;
	uint8_t block_erase_ext_command_supported : 1;
	uint32_t user_addressable_sectors;
	uint16_t obsolete_word62;
	uint16_t multi_word_dma_support : 8;
	uint16_t multi_word_dma_active : 8;
	uint16_t advanced_pio_modes : 8;
	uint16_t reserved_byte64 : 8;
	uint16_t minimum_mxxfer_cycle_time;
	uint16_t recommended_mwxfer_cycle_time;
	uint16_t minimum_pio_cycle_time;
	uint16_t minimum_pio_cycle_time_iordy;
	struct
	{
		uint16_t zoned_capabilities : 2;
		uint16_t non_volatile_write_cache : 1;
		uint16_t extended_user_addressable_sectors_supported : 1;
		uint16_t device_encrypts_all_user_data : 1;
		uint16_t read_zero_after_trim_supported : 1;
		uint16_t optional_28bit_commands_supported : 1;
		uint16_t ieee1667 : 1;
		uint16_t download_microcode_dma_supported : 1;
		uint16_t set_max_set_password_unlock_dma_supported : 1;
		uint16_t write_buffer_dma_supported : 1;
		uint16_t read_buffer_dma_supported : 1;
		uint16_t device_config_identify_set_dma_supported : 1;
		uint16_t lpsaerc_supported : 1;
		uint16_t deterministic_read_after_trim_supported : 1;
		uint16_t cfast_spec_supported : 1;
	} additional_supported;
	uint16_t reserved_words70[5];
	uint16_t queue_depth : 5;
	uint16_t reserved_word75 : 11;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t sata_gen1 : 1;
		uint16_t sata_gen2 : 1;
		uint16_t sata_gen3 : 1;
		uint16_t reserved1 : 4;
		uint16_t ncq : 1;
		uint16_t hipm : 1;
		uint16_t phy_events : 1;
		uint16_t ncq_unload : 1;
		uint16_t ncq_priority : 1;
		uint16_t host_auto_ps : 1;
		uint16_t device_auto_ps : 1;
		uint16_t read_log_dma : 1;
		uint16_t reserved2 : 1;
		uint16_t current_speed : 3;
		uint16_t ncq_streaming : 1;
		uint16_t ncq_queue_mgmt : 1;
		uint16_t ncq_receive_send : 1;
		uint16_t devsl_pto_reduced_pwr_state : 1;
		uint16_t reserved3 : 8;
	} serial_ata_capabilities;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t non_zero_offsets : 1;
		uint16_t dma_setup_auto_activate : 1;
		uint16_t dipm : 1;
		uint16_t in_order_data : 1;
		uint16_t hardware_feature_control : 1;
		uint16_t software_settings_preservation : 1;
		uint16_t ncq_autosense : 1;
		uint16_t devslp : 1;
		uint16_t hybrid_information : 1;
		uint16_t reserved1 : 6;
	} serial_ata_features_supported;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t non_zero_offsets : 1;
		uint16_t dma_setup_auto_activate : 1;
		uint16_t dpim : 1;
		uint16_t in_order_data : 1;
		uint16_t hardware_feature_control : 1;
		uint16_t software_settings_preservation : 1;
		uint16_t device_auto_ps : 1;
		uint16_t devslp : 1;
		uint16_t hybrid_information : 1;
		uint16_t reserved1 : 6;
	} serial_ata_features_enabled;
	uint16_t major_revision;
	uint16_t minor_revision;
	struct
	{
		uint16_t smart_commands : 1;
		uint16_t security_mode : 1;
		uint16_t removable_media_feature : 1;
		uint16_t power_management : 1;
		uint16_t reserved1 : 1;
		uint16_t write_cache : 1;
		uint16_t look_ahead : 1;
		uint16_t release_interrupt : 1;
		uint16_t service_interrupt : 1;
		uint16_t device_reset : 1;
		uint16_t host_protected_area : 1;
		uint16_t obsolete1 : 1;
		uint16_t write_buffer : 1;
		uint16_t read_buffer : 1;
		uint16_t nop : 1;
		uint16_t obsolete2 : 1;
		uint16_t download_microcode : 1;
		uint16_t dma_queued : 1;
		uint16_t cfa : 1;
		uint16_t advanced_pm : 1;
		uint16_t msn : 1;
		uint16_t power_up_in_standby : 1;
		uint16_t manual_power_up : 1;
		uint16_t reserved2 : 1;
		uint16_t set_max : 1;
		uint16_t acoustics : 1;
		uint16_t big_lba : 1;
		uint16_t device_config_overlay : 1;
		uint16_t flush_cache : 1;
		uint16_t flush_cache_ext : 1;
		uint16_t word_valid83 : 2;
		uint16_t smart_error_log : 1;
		uint16_t smart_self_test : 1;
		uint16_t media_serial_number : 1;
		uint16_t media_card_pass_through : 1;
		uint16_t streaming_feature : 1;
		uint16_t gp_logging : 1;
		uint16_t write_fua : 1;
		uint16_t write_queued_fua : 1;
		uint16_t wwm_64bit : 1;
		uint16_t urg_read_stream : 1;
		uint16_t urg_write_stream : 1;
		uint16_t reserved_for_tech_report : 2;
		uint16_t idle_with_unload_feature : 1;
		uint16_t wordValid : 2;
	} command_set_support;
	struct
	{
		uint16_t smart_commands : 1;
		uint16_t security_mode : 1;
		uint16_t removable_media_feature : 1;
		uint16_t power_management : 1;
		uint16_t reserved1 : 1;
		uint16_t write_cache : 1;
		uint16_t look_ahead : 1;
		uint16_t release_interrupt : 1;
		uint16_t service_interrupt : 1;
		uint16_t device_reset : 1;
		uint16_t host_protected_area : 1;
		uint16_t obsolete1 : 1;
		uint16_t write_buffer : 1;
		uint16_t read_buffer : 1;
		uint16_t nop : 1;
		uint16_t obsolete2 : 1;
		uint16_t download_microcode : 1;
		uint16_t dma_queued : 1;
		uint16_t cfa : 1;
		uint16_t advanced_pm : 1;
		uint16_t msn : 1;
		uint16_t power_up_in_standby : 1;
		uint16_t manual_power_up : 1;
		uint16_t reserved2 : 1;
		uint16_t set_max : 1;
		uint16_t acoustics : 1;
		uint16_t big_lba : 1;
		uint16_t device_config_overlay : 1;
		uint16_t flush_cache : 1;
		uint16_t flush_cache_ext : 1;
		uint16_t reserved3 : 1;
		uint16_t words119_120_valid : 1;
		uint16_t smart_error_log : 1;
		uint16_t smart_self_test : 1;
		uint16_t media_serial_number : 1;
		uint16_t media_card_pass_through : 1;
		uint16_t streaming_feature : 1;
		uint16_t gpLogging : 1;
		uint16_t write_fua : 1;
		uint16_t write_queued_fua : 1;
		uint16_t wwm_64bit : 1;
		uint16_t urg_read_stream : 1;
		uint16_t urg_write_stream : 1;
		uint16_t reserved_for_tech_report : 2;
		uint16_t idle_with_unload_feature : 1;
		uint16_t reserved4 : 2;
	} command_set_active;
	uint16_t ultradma_support : 8;
	uint16_t ultradma_active : 8;
	struct
	{
		uint16_t time_required : 15;
		uint16_t extended_time_reported : 1;
	} normal_security_erase_unit;
	struct
	{
		uint16_t time_required : 15;
		uint16_t extended_time_reported : 1;
	} enhanced_security_erase_unit;
	uint16_t current_apm_level : 8;
	uint16_t reserved_word91 : 8;
	uint16_t master_password_id;
	uint16_t hardware_reset_result;
	uint16_t current_acoustic_value : 8;
	uint16_t recommended_acoustic_value : 8;
	uint16_t stream_min_request_size;
	uint16_t streaming_transfer_time_dma;
	uint16_t streaming_access_latency_dmapio;
	uint32_t streaming_perf_granularity;
	uint32_t max_48bit_lba[2];
	uint16_t streaming_transfer_time;
	uint16_t dsm_cap;
	struct
	{
		uint16_t logical_sectors_per_physical_sector : 4;
		uint16_t reserved0 : 8;
		uint16_t logical_sector_longer_than256_words : 1;
		uint16_t multiple_logical_sectors_per_physical_sector : 1;
		uint16_t reserved1 : 2;
	} physical_logical_sector_size;
	uint16_t inter_seek_delay;
	uint16_t world_wide_name[4];
	uint16_t reserved_for_world_wide_name128[4];
	uint16_t reserved_for_tlc_technical_report;
	uint16_t words_per_logical_sector[2];
	struct
	{
		uint16_t reserved_for_drq_technical_report : 1;
		uint16_t write_read_verify : 1;
		uint16_t write_uncorrectable_ext : 1;
		uint16_t read_write_log_dma_ext : 1;
		uint16_t download_microcode_mode3 : 1;
		uint16_t freefall_control : 1;
		uint16_t sense_data_reporting : 1;
		uint16_t extended_power_conditions : 1;
		uint16_t reserved0 : 6;
		uint16_t word_valid : 2;
	} command_set_support_ext;
	struct
	{
		uint16_t reserved_for_drq_technical_report : 1;
		uint16_t write_read_verify : 1;
		uint16_t write_uncorrectable_ext : 1;
		uint16_t read_write_log_dma_ext : 1;
		uint16_t download_microcode_mode3 : 1;
		uint16_t freefall_control : 1;
		uint16_t sense_data_reporting : 1;
		uint16_t extended_power_conditions : 1;
		uint16_t reserved0 : 6;
		uint16_t reserved1 : 2;
	} command_set_active_ext;
	uint16_t reserved_for_expanded_supportand_active[6];
	uint16_t msn_support : 2;
	uint16_t reserved_word127 : 14;
	struct
	{
		uint16_t security_supported : 1;
		uint16_t security_enabled : 1;
		uint16_t security_locked : 1;
		uint16_t security_frozen : 1;
		uint16_t security_count_expired : 1;
		uint16_t enhanced_security_erase_supported : 1;
		uint16_t reserved0 : 2;
		uint16_t security_level : 1;
		uint16_t reserved1 : 7;
	} security_status;
	uint16_t reserved_word129[31];
	struct
	{
		uint16_t maximum_current_in_ma : 12;
		uint16_t cfa_power_mode1_disabled : 1;
		uint16_t cfa_power_mode1_required : 1;
		uint16_t reserved0 : 1;
		uint16_t word160_supported : 1;
	} cfa_power_mode1;
	uint16_t reserved_for_cfa_word161[7];
	uint16_t nominal_form_factor : 4;
	uint16_t reserved_word168 : 12;
	struct
	{
		uint16_t supports_trim : 1;
		uint16_t reserved0 : 15;
	} data_set_management_feature;
	uint16_t additional_product_id[4];
	uint16_t reserved_for_cfa_word174[2];
	uint16_t current_media_serial_number[30];
	struct
	{
		uint16_t supported : 1;
		uint16_t reserved0 : 1;
		uint16_t write_same_supported : 1;
		uint16_t error_recovery_control_supported : 1;
		uint16_t feature_control_supported : 1;
		uint16_t data_tables_supported : 1;
		uint16_t reserved1 : 6;
		uint16_t vendor_specific : 4;
	} sct_command_transport;
	uint16_t reserved_word207[2];
	struct
	{
		uint16_t alignment_of_logical_within_physical : 14;
		uint16_t word209_supported : 1;
		uint16_t reserved0 : 1;
	} block_alignment;
	uint16_t write_read_verify_sector_count_mode3_only[2];
	uint16_t write_read_verify_sector_count_mode2_only[2];
	struct
	{
		uint16_t nv_cache_power_mode_enabled : 1;
		uint16_t reserved0 : 3;
		uint16_t nv_cache_feature_set_enabled : 1;
		uint16_t Reserved1 : 3;
		uint16_t nv_cache_power_mode_version : 4;
		uint16_t nv_cache_feature_set_version : 4;
	} nv_cache_capabilities;
	uint16_t nv_cache_size_lsw;
	uint16_t nv_cache_size_msw;
	uint16_t nominal_media_rotation_rate;
	uint16_t reserved_word218;
	struct
	{
		uint8_t nv_cache_estimated_time_to_spin_up_in_seconds;
		uint8_t reserved;
	} nv_cache_options;
	uint16_t write_read_verify_sector_count_mode : 8;
	uint16_t reserved_word220 : 8;
	uint16_t reserved_word221;
	struct
	{
		uint16_t major_version : 12;
		uint16_t transport_type : 4;
	} transport_major_version;
	uint16_t transport_minor_version;
	uint16_t reserved_word224[6];
	uint32_t extended_number_of_user_addressable_sectors[2];
	uint16_t min_blocks_per_download_microcode_mode03;
	uint16_t max_blocks_per_download_microcode_mode03;
	uint16_t reserved_word236[19];
	uint16_t signature : 8;
	uint16_t checksum : 8;
} __attribute((packed));

struct ata_identify_packet
{
	struct
	{
		uint16_t packet_type : 2;
		uint16_t incomplete_response : 1;
		uint16_t reserved1 : 2;
		uint16_t drq_delay : 2;
		uint16_t removable_media : 1;
		uint16_t command_packet_type : 5;
		uint16_t reserved2 : 1;
		uint16_t device_type : 2;
	} general_configuration;
	uint16_t reseved_word1;
	uint16_t unique_configuration;
	uint16_t reserved_words3[7];
	uint8_t serial_number[20];
	uint16_t reserved_words20[3];
	uint8_t firmware_revision[8];
	uint8_t model_number[40];
	uint16_t reserved_words47[2];
	struct
	{
		uint16_t vendor_specific : 8;
		uint16_t dma_supported : 1;
		uint16_t lba_supported : 1;
		uint16_t iordy_disabled : 1;
		uint16_t iordy_supported : 1;
		uint16_t obsolete : 1;
		uint16_t overlap_supported : 1;
		uint16_t oueued_commands_supported : 1;
		uint16_t interleaved_dma_supported : 1;
		uint16_t device_specific_standby_timer_value_min : 1;
		uint16_t obsolete1 : 1;
		uint16_t reserved_word50 : 12;
		uint16_t word_valid : 2;
	} capabilities;
	uint16_t obsolete_words51[2];
	uint16_t translation_fields_valid : 3;
	uint16_t reserved3 : 13;
	uint16_t reserved_words54[8];
	struct
	{
		uint16_t udma0_supported : 1;
		uint16_t udma1_supported : 1;
		uint16_t udma2_supported : 1;
		uint16_t udma3_supported : 1;
		uint16_t udma4_supported : 1;
		uint16_t udma5_supported : 1;
		uint16_t udma6_supported : 1;
		uint16_t mdma0_supported : 1;
		uint16_t mdma1_supported : 1;
		uint16_t mdma2_supported : 1;
		uint16_t dma_supported : 1;
		uint16_t reserved_word62 : 4;
		uint16_t dmadir_bit_required : 1;
	} dmadir;
	uint16_t multi_word_dma_support : 8;
	uint16_t multi_word_dma_active : 8;
	uint16_t advanced_pio_modes : 8;
	uint16_t reserved_byte64 : 8;
	uint16_t minimum_mwxfer_cycle_time;
	uint16_t recommended_mwxfer_cycle_time;
	uint16_t minimum_pio_cycle_time;
	uint16_t minimum_pio_cycle_time_iordy;
	uint16_t reserved_words69[2];
	uint16_t bus_release_delay;
	uint16_t service_command_delay;
	uint16_t reserved_words73[2];
	uint16_t queue_depth : 5;
	uint16_t reserved_word75 : 11;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t sata_gen1 : 1;
		uint16_t sata_gen2 : 1;
		uint16_t sata_gen3 : 1;
		uint16_t reserved1 : 5;
		uint16_t hipm : 1;
		uint16_t phy_events : 1;
		uint16_t reserved3 : 2;
		uint16_t host_autoPS : 1;
		uint16_t device_autoPS : 1;
		uint16_t reserved4 : 1;
		uint16_t reserved5 : 1;
		uint16_t current_speed : 3;
		uint16_t slimline_device_attention : 1;
		uint16_t host_environment_detect : 1;
		uint16_t reserved : 10;
	} serial_ata_capabilities;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t reserved1 : 2;
		uint16_t dipm : 1;
		uint16_t reserved2 : 1;
		uint16_t asynchronous_notification : 1;
		uint16_t software_settings_preservation : 1;
		uint16_t reserved3 : 9;
	} serial_ata_features_supported;
	struct
	{
		uint16_t reserved0 : 1;
		uint16_t reserved1 : 2;
		uint16_t dipm : 1;
		uint16_t reserved2 : 1;
		uint16_t asynchronous_notification : 1;
		uint16_t software_settings_preservation : 1;
		uint16_t device_auto_ps : 1;
		uint16_t reserved3 : 8;
	} serial_ata_features_enabled;
	uint16_t major_revision;
	uint16_t minor_revision;
	struct
	{
		uint16_t smart_commands : 1;
		uint16_t security_mode : 1;
		uint16_t removable_media : 1;
		uint16_t power_management : 1;
		uint16_t packet_commands : 1;
		uint16_t write_cache : 1;
		uint16_t look_ahead : 1;
		uint16_t release_interrupt : 1;
		uint16_t service_interrupt : 1;
		uint16_t device_reset : 1;
		uint16_t host_protected_area : 1;
		uint16_t obsolete1 : 1;
		uint16_t write_buffer : 1;
		uint16_t read_buffer : 1;
		uint16_t nop : 1;
		uint16_t obsolete2 : 1;
		uint16_t download_microcode : 1;
		uint16_t reserved1 : 2;
		uint16_t advanced_pm : 1;
		uint16_t msn : 1;
		uint16_t power_up_in_standby : 1;
		uint16_t manual_power_up : 1;
		uint16_t reserved2 : 1;
		uint16_t set_max : 1;
		uint16_t reserved3 : 3;
		uint16_t flush_cache : 1;
		uint16_t reserved4 : 1;
		uint16_t word_valid : 2;
	} command_set_support;
	struct
	{
		uint16_t reserved0 : 5;
		uint16_t gp_logging : 1;
		uint16_t reserved1 : 2;
		uint16_t wwn_64bit : 1;
		uint16_t reserved2 : 5;
		uint16_t word_valid : 2;
	} command_set_support_ext;
	struct
	{
		uint16_t smart_commands : 1;
		uint16_t security_mode : 1;
		uint16_t removable_media : 1;
		uint16_t power_management : 1;
		uint16_t packet_commands : 1;
		uint16_t write_cache : 1;
		uint16_t look_ahead : 1;
		uint16_t release_interrupt : 1;
		uint16_t service_interrupt : 1;
		uint16_t device_reset : 1;
		uint16_t host_protected_area : 1;
		uint16_t obsolete1 : 1;
		uint16_t write_buffer : 1;
		uint16_t read_buffer : 1;
		uint16_t nop : 1;
		uint16_t obsolete2 : 1;
		uint16_t download_microcode : 1;
		uint16_t reserved1 : 2;
		uint16_t advanced_pm : 1;
		uint16_t msn : 1;
		uint16_t power_up_in_standby : 1;
		uint16_t manual_powerUp : 1;
		uint16_t reserved2 : 1;
		uint16_t set_max : 1;
		uint16_t reserved3 : 3;
		uint16_t flush_cache : 1;
		uint16_t reserved : 3;
	} command_set_active;
	struct
	{
		uint16_t reserved0 : 5;
		uint16_t gp_logging : 1;
		uint16_t reserved1 : 2;
		uint16_t wwm_64bit : 1;
		uint16_t reserved2 : 5;
		uint16_t word_valid : 2;
	} command_set_active_ext;
	uint16_t ultra_dma_support : 8;
	uint16_t ultra_dma_active : 8;
	uint16_t time_required_for_normal_erase_mode_security_erase_unit;
	uint16_t time_required_for_enhanced_erase_mode_security_erase_unit;
	uint16_t current_apm_level;
	uint16_t master_password_id;
	uint16_t hardware_reset_result;
	uint16_t reserved_words94[14];
	uint16_t world_wide_name[4];
	uint16_t reserved_words112[13];
	uint16_t atapi_zero_byte_count;
	uint16_t reserved_word126;
	uint16_t msn_support : 2;
	uint16_t reserved_word127 : 14;
	uint16_t security_status;
	uint16_t vendor_specific[31];
	uint16_t reserved_word160[16];
	uint16_t reserved_word176[46];
	struct
	{
		uint16_t major_version : 12;
		uint16_t transport_type : 4;
	} transport_major_version;
	uint16_t transport_minor_version;
	uint16_t reserved_word224[31];
	uint16_t signature : 8;
	uint16_t checksum : 8;
} __attribute((packed));

#endif
