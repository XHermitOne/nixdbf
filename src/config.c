/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
* @version 0.0.0.1
*/

#include "config.h"

/**
* Основные настройки
*/

char CfgPath[] = ".nixdbf";           /**< Папка конфигурациронных файлов*/
char CfgFileName[] = "nixdbf.cfg";    /**< Имя файла конфигурации основных параметров*/
static char FullCfgFileName[MAX_PATH];

nix_config_t config_strings[] =
{
    { "TEST",                     ""},
};


BOOL save_config(char *cfg_filename, nix_config_t *strings, int count)
{
    int i = 0;
    FILE *f;

    if (DebugMode) log_line("Saving config settings %s %i strings.\n", cfg_filename, count);

    f = fopen(cfg_filename, "wt");
    if (!f)
    {
        if (DebugMode) log_line("ERROR: Can't write the file %s \n", FullCfgFileName);
        return FALSE;   // can't write the file, but don't complain
    }

    for (i = 0; i < count; i++)
    {
        fprintf(f, "%s\t\t\"%s\"\n", strings[i].name, strings[i].location);
    }

    fclose (f);
    return TRUE;
}


BOOL load_config(const char *cfg_filename, nix_config_t *strings, int count)
{
    int i = 0;
    int len = 0;
    FILE *f = NULL;
    char def[STD_STR_LEN];
    char strparm[100];
    int parm = 0;

    if (DebugMode) log_line("Loading config settings %s %i strings.\n", cfg_filename, count);

    f = fopen(cfg_filename, "rt");
    if (f)
    {
        while (!feof(f))
        {
            if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
            {
                if (DebugMode) log_line("Line: %s %s.\n", def, strparm);
                if (strparm[0] == '"')  /* string values */
                {
                    for (i = 0; i < count; i++)
                    {
                        if (!strcmp(def, strings[i].name))
                        {
                            len = (int) strlen(strparm) - 2;
                            if (len <= 0)
                            {
                                strings[i].location[0] = '\0';
                                break;
                            }
                            if (len > 79)  len = 79;
                            strncpy(strings[i].location, strparm + 1, len);
                            strings[i].location[len] = '\0';
                            if (DebugMode) log_line("Name: %s Value: %s.\n", strings[i].name, strings[i].location);
                            break;
                        }
                    }
                continue;
                }
            }
        }
        fclose (f);
    }
    else
    {
        if (DebugMode) log_line("ERROR: Cant read file %s \n", cfg_filename);
        return FALSE;
    }
    return TRUE;
}


static BOOL create_config_path(char *Path);

char *get_config_path(void)
{
    char *cfg_path = NULL;
    char *home_path = get_home_path();

    cfg_path = (char*) calloc(strlen(home_path) + 1 + strlen(CfgPath) + 1, sizeof(char));
    strcpy(cfg_path, home_path);
    strcat(cfg_path, "/");
    strcat(cfg_path, CfgPath);

    if (!dir_exists(cfg_path))
        create_config_path(cfg_path);

    if (DebugMode) log_line("GetCfgPath: %s\n", cfg_path);

    return cfg_path;
}

/**
*   Сгенерировать путь к конфигурационному файлу.
*   По умолчанию Path = NULL
*/
static BOOL create_config_path(char *path)
{
    BOOL do_free = FALSE;
    char *cfg_path = path;

    if (cfg_path == NULL)
    {
        cfg_path = get_config_path();
        do_free = TRUE;
    }

    BOOL result = (BOOL) mkpath(cfg_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if(DebugMode) log_line("Create path: %s\n", cfg_path);

    if (do_free)
        free(cfg_path);

    return result;
}


static char *get_config_filename(char *filename)
{
    char *cfg_filename = NULL;
    char *cfg_path = get_config_path();

    if (cfg_filename != NULL)
    {
        cfg_filename = (char*)calloc(strlen(filename) + 1, sizeof(char));
        strcpy(cfg_filename, filename);
    }
    else
    {
        cfg_filename = (char*)calloc(strlen(cfg_path) + 1 + strlen(CfgFileName) + 1, sizeof(char));
        strcpy(cfg_filename, cfg_path);
        strcat(cfg_filename, "/");
        strcat(cfg_filename, CfgFileName);
    }

    free(cfg_path);

    if(DebugMode) log_line("GetCfgFileName: %s\n", cfg_filename);
    return cfg_filename;
}
