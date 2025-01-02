import model
from util import world_version_is_wrath, is_cpp
from writer import Writer

def print_achievement_done_array_c(s: Writer, h: Writer):
    h.write_block(f"""
        typedef struct {{
            uint32_t amount_of_achievements;
            wrath_AchievementDone* achievements;
        }} wrath_AchievementDoneArray;
    """)

    s.write_block(f"""
        static WowWorldResult wrath_achievement_done_array_write(WowWorldWriter* stream, const wrath_AchievementDoneArray* mask) {{
            int _return_value = 1;
            uint32_t i;
            for (i = 0; i < mask->amount_of_achievements; ++i) {{
                WWM_CHECK_RETURN_CODE(wrath_AchievementDone_write(stream, &mask->achievements[i]));
            }}

            WWM_CHECK_RETURN_CODE(wwm_write_u32(stream, 0xFFFFFFFF));

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static WowWorldResult wrath_achievement_done_array_read(WowWorldReader* stream, wrath_AchievementDoneArray* mask) {{
            int _return_value = 1;
            uint32_t achievement;
            size_t array_size = 8;

            WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &achievement));
            stream->index -= 4; /* we're just checking achievement */

            mask->achievements = malloc(array_size * sizeof(wrath_AchievementDone));
            if (mask->achievements == NULL) {{
                return WWM_RESULT_MALLOC_FAIL;
            }}
            mask->amount_of_achievements = 0;

            while(achievement != 0xFFFFFFFF) {{
                mask->achievements[mask->amount_of_achievements].achievement = achievement;
                WWM_CHECK_RETURN_CODE(wrath_AchievementDone_read(stream, &mask->achievements[mask->amount_of_achievements]));

                WWM_CHECK_RETURN_CODE(wwm_read_u32(stream, &achievement));
                ++mask->amount_of_achievements;
                stream->index -= 4; /* we're just checking achievement */

                if (mask->amount_of_achievements > array_size) {{
                    array_size *= 2;
                    mask->achievements = realloc(mask->achievements, array_size * sizeof(wrath_AchievementDone));
                    if (mask->achievements == NULL) {{
                        return WWM_RESULT_MALLOC_FAIL;
                    }}
                }}
            }}

            return WWM_RESULT_SUCCESS;
        cleanup: return _return_value;
        }}

        static size_t wrath_achievement_done_array_size(const wrath_AchievementDoneArray* mask) {{
            return mask->amount_of_achievements * 8;
        }}

        static void wrath_achievement_done_array_free(wrath_AchievementDoneArray* mask) {{
            mask->amount_of_achievements = 0;
            free(mask->achievements);
        }}
    """)

def print_achievement_done_array_cpp(s: Writer, h: Writer):
    s.write_block(f"""
        static void achievement_done_array_write(Writer& writer, const std::vector<AchievementDone>& mask) {{
            for (const auto& v : mask) {{
                AchievementDone_write(writer, v);
            }}

            writer.write_u32(0xFFFFFFFF);
        }}

        static std::vector<AchievementDone> achievement_done_array_read(Reader& reader) {{
            std::vector<AchievementDone> mask;
            uint32_t achievement = reader.read_u32();

            while(achievement != 0xFFFFFFFF) {{
                uint32_t time = reader.read_u32();

                mask.push_back({{achievement, time}});

                achievement = reader.read_u32();
            }}

            return mask;
        }}

        static size_t achievement_done_array_size(const std::vector<AchievementDone>& mask) {{
            return mask.size() * 8;
        }}
    """)

def print_achievement_done_array(s: Writer, h: Writer, v: model.WorldVersion):
    if not world_version_is_wrath(v):
        return
    if is_cpp():
        print_achievement_done_array_cpp(s, h)
    else:
        print_achievement_done_array_c(s, h)
