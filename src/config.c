/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
*/

#include "config.h"

/**
* Основные настройки
*/

char CfgPath[] = ".nixdbf";           /**< Папка конфигурациронных файлов*/
char CfgFileName[] = "nixdbf.cfg";    /**< Имя файла конфигурации основных параметров*/
static char FullCfgFileName[MAX_PATH];

Config_t config_strings[] =
{
	{ "TEST",                     ""},
};


BOOL saveConfig (char *CFGFileName, Config_t *Strings, int Count)
{
	int	i = 0;
	FILE *f;

	if(DBG_MODE) logInfo("Saving config settings %s %i strings.\n", CFGFileName, Count);
    
	f = fopen (CFGFileName, "wt");
	if (!f)
    {
        if(DBG_MODE) logInfo("ERROR: Can't write the file %s \n", FullCfgFileName);
		return FALSE;	// can't write the file, but don't complain
    }

	for (i = 0; i < Count; i++)
	{
		fprintf(f, "%s\t\t\"%s\"\n", Strings[i].name, Strings[i].location);
	}

	fclose (f);
    return TRUE;
}


BOOL loadConfig(const char *CFGFileName, Config_t *Strings, int Count)
{
	int i = 0; 
	int len = 0;
	FILE *f = NULL;
	char def[STD_STR_LEN];
	char strparm[100];
	int parm = 0;

	if(DBG_MODE) logInfo("Loading config settings %s %i strings.\n", CFGFileName, Count);

    f = fopen(CFGFileName, "rt");
	if (f)
	{
		while (!feof(f))
		{
			if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
			{
                if(DBG_MODE) logInfo("Line: %s %s.\n", def, strparm);                
				if (strparm[0] == '"') /* string values */
				{
                    for (i=0; i<Count; i++)
                    {
                        if (!strcmp(def, Strings[i].name))
                        {
                            len = (int)strlen(strparm) - 2;
                            if (len <= 0)
                            {
                                Strings[i].location[0] = '\0';
                                break;
                            }
                            if (len > 79)  len = 79;
                            strncpy(Strings[i].location, strparm + 1, len);
                            Strings[i].location[len] = '\0';
                            if(DBG_MODE) logInfo("Name: %s Value: %s.\n", Strings[i].name, Strings[i].location);                
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
        if(DBG_MODE) logInfo("ERROR: Cant read file %s \n", CFGFileName);
        return FALSE;
    }
    return TRUE;
}


static BOOL createCfgPath(char *Path);
char *getCfgPath(void)
{
    char *result = NULL;
    char *home_path = getHomePath();
    
    result = (char*)calloc(strlen(home_path)+1+strlen(CfgPath)+1, sizeof(char));
    strcpy(result, home_path);
    strcat(result, "/");
    strcat(result, CfgPath);

    if (!dir_exists(result)) 
        createCfgPath(result);

    if(DBG_MODE) logInfo("GetCfgPath: %s\n", result);
    
    return result;
}

/**
*   Сгенерировать путь к конфигурационному файлу.
*   По умолчанию Path=NULL
*/
static BOOL createCfgPath(char *Path)
{
    int do_free = 0;
    char *conf_path = Path;

    if (conf_path == NULL)
    {
        conf_path = getCfgPath();
        do_free = 1;
    }
        
    BOOL result = (BOOL) mkpath(conf_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if(DBG_MODE) logInfo("Create path: %s\n", conf_path);

    if (do_free)
        free(conf_path);    
        
    return result;
}


static char *getCfgFileName(char *FileName)
{
    char *filename = NULL;
    char *conf_path = getCfgPath();

    if (FileName != NULL)
    {
        filename = (char*)calloc(strlen(FileName)+1, sizeof(char));
        strcpy(filename, FileName);
    }
    else
    {
        filename = (char*)calloc(strlen(conf_path)+1+strlen(CfgFileName)+1, sizeof(char));
        strcpy(filename, conf_path);
        strcat(filename, "/");
        strcat(filename, CfgFileName);
    }

    free(conf_path);
    
    if(DBG_MODE) logInfo("GetCfgFileName: %s\n", filename);
    return filename;
}
