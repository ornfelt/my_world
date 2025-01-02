import model
from util import world_version_is_wrath, is_cpp
from writer import Writer

def print_achievement_in_progress_array_c(s: Writer, h: Writer):
    h.write_block(f"""
        typedef struct {{
            uint32_t amount_of_achievements;
            wrath_AchievementInProgress* achievements;
        }} wrath_AchievementInProgressArray;
    """)

    s.write_block(f"""
        static WowWorldResult wrath_achievement_in_progress_array_write(WowWorldWriter* stream, const wrath_AchievementInProgressArray* mask) {{
            int _return_value = 1;
            uint32_t i;
            for (i = 0; i < mask->amount_of_achievements; ++i) {{
                WWM_CHECK_RETURN_CODE(wrath_AchievementInProgress_write(stream, &mask->achievements[i]));
            }}

            WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, 0xFFFFFFFF));

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static WowWorldResult wrath_achievement_in_progress_array_read(WowWorldReader* stream, wrath_AchievementInProgressArray* mask) {{
            int _return_value = 1;
            uint32_t achievement;
            size_t array_size = 8;

            WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &achievement));
            stream->index -= 4; /* we're just checking achievement */

            mask->achievements = malloc(array_size * sizeof(wrath_AchievementInProgress));
            if (mask->achievements == NULL) {{
                return WWM_RESULT_MALLOC_FAIL;
            }}
            mask->amount_of_achievements = 0;

            while(achievement != 0xFFFFFFFF) {{
                mask->achievements[mask->amount_of_achievements].achievement = achievement;
                WWM_CHECK_RETURN_CODE(wrath_AchievementInProgress_read(stream, &mask->achievements[mask->amount_of_achievements]));

                WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &achievement));
                stream->index -= 4; /* we're just checking achievement */
                ++mask->amount_of_achievements;

                if (mask->amount_of_achievements > array_size) {{
                    array_size *= 2;
                    mask->achievements = realloc(mask->achievements, array_size * sizeof(wrath_AchievementInProgress));
                    if (mask->achievements == NULL) {{
                        return WWM_RESULT_MALLOC_FAIL;
                    }}
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static size_t wrath_achievement_in_progress_array_size(const wrath_AchievementInProgressArray* mask) {{
            size_t size = 0;
            uint32_t i;
            for (i = 0; i < mask->amount_of_achievements; ++i) {{
                size += wrath_AchievementInProgress_size(&mask->achievements[i]);
            }}
            return size;
        }}

        static void wrath_achievement_in_progress_array_free(wrath_AchievementInProgressArray* mask) {{
            mask->amount_of_achievements = 0;
            free(mask->achievements);
        }}
    """)

def print_achievement_in_progress_array_cpp(s: Writer, h: Writer):
    s.write_block(f"""
        static void achievement_in_progress_array_write(Writer& writer, const std::vector<AchievementInProgress>& mask) {{
            for (const auto& v : mask) {{
                AchievementInProgress_write(writer, v);
            }}

            writer.write_u32(0xFFFFFFFF);
        }}

        static std::vector<AchievementInProgress> achievement_in_progress_array_read(Reader& reader) {{
            std::vector<AchievementInProgress> mask;
            uint32_t achievement = reader.read_u32();

            while(achievement != 0xFFFFFFFF) {{
                AchievementInProgress obj;

                obj.achievement = achievement;

                obj.counter = reader.read_packed_guid();

                obj.player = reader.read_packed_guid();

                obj.timed_criteria_failed = reader.read_bool32();

                obj.progress_date = reader.read_u32();

                obj.time_since_progress = reader.read_u32();

                obj.time_since_progress2 = reader.read_u32();

                mask.push_back(obj);

                achievement = reader.read_u32();
            }}

            return mask;
        }}

        static size_t achievement_in_progress_array_size(const std::vector<AchievementInProgress>& mask) {{
            size_t size = 0;
            for (const auto& v : mask) {{
                size += AchievementInProgress_size(v);
            }}
            return size;
        }}
    """)

def print_achievement_in_progress_array(s: Writer, h: Writer, v: model.WorldVersion):
    if not world_version_is_wrath(v):
        return
    if is_cpp():
        print_achievement_in_progress_array_cpp(s, h)
    else:
        print_achievement_in_progress_array_c(s, h)
