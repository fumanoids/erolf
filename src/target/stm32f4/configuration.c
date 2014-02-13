/*
 * configuration.c
 *
 *  Created on: 05.05.2012
 *      Author: danielb
 */

#include <libopencm3/stm32/f4/rcc.h>
#include <flawless/init/systemInitializer.h>

#include <interfaces/configuration.h>

#include <libopencm3/stm32/f4/flash.h>
#include <string.h>

extern uint16_t _flashConfigROMBegin;
extern uint16_t _flashConfigROMEnd;

extern const configVariableDescriptor_t _applicationConfigDescriptorsBegin, _applicationConfigDescriptorsEnd;

#define FLASH_CONFIG_SECTOR (11 << 3)
#define FLASH_PROGRAMM_ACCESS_SIZE (1 << 8)

#define FLASH_CONFIG_SECTOR_SIZE (128 * 1024)
#define FLASH_CONFIG_START_ADDR   (0x080e0000)

#define IS_POINTER_IN_CONFIG_MEMORY(ptr) (((void*)(ptr) >= (void*)&_flashConfigROMBegin) && ((void*)(ptr) < (void*)&_flashConfigROMEnd))

typedef struct tag_configRecordEntry
{
	uint32_t nextEntryOffset;
	uint32_t nameOffset;
	uint32_t filenameOffset;
	uint32_t dataOffset;
	uint32_t dataCnt;
}configRecordEntry_t;


static void readCurrentConfigurationFromFlash()
{

	uint16_t i = 0U;

	const configVariableDescriptor_t *descriptor = &_applicationConfigDescriptorsBegin;
	const uint16_t descriptorsCnt = ((uint32_t)&_applicationConfigDescriptorsEnd - (uint32_t)&_applicationConfigDescriptorsBegin) / sizeof(configVariableDescriptor_t);

	/* for every descriptor */
	for (i = 0U; i < descriptorsCnt; ++i)
	{
		/* find the corresponding space in the configuration area */

		const configRecordEntry_t *curRecord = (const configRecordEntry_t *)&_flashConfigROMBegin;

		/* pointer to a memory region to retrieve the configuration data for this descriptor from */
		const void *configDataLoadMemory = descriptor[i].defaultValuesRegion;

		while (NULL != curRecord &&
				IS_POINTER_IN_CONFIG_MEMORY(curRecord))
		{
			if ((0 == curRecord->dataCnt)         || ((uint32_t)~0 == curRecord->dataCnt)         ||
				(0 == curRecord->dataOffset)      || ((uint32_t)~0 == curRecord->dataOffset)      ||
				(0 == curRecord->filenameOffset)  || ((uint32_t)~0 == curRecord->filenameOffset)  ||
				(0 == curRecord->nameOffset)      || ((uint32_t)~0 == curRecord->nameOffset)      ||
				(0 == curRecord->nextEntryOffset) || ((uint32_t)~0 == curRecord->nextEntryOffset))
			{
				/* this is the guard keeper or just an invalid entry (marking the end of all records) */
				/* get out of the loop */
				break;
			} else
			{
				/* test if this is the matching record in the configuration */
				const char *nameStrPtr     = ((const char*) curRecord) + curRecord->nameOffset;
				const char *fileNameStrPtr = ((const char*) curRecord) + curRecord->filenameOffset;
				if (IS_POINTER_IN_CONFIG_MEMORY(nameStrPtr) && IS_POINTER_IN_CONFIG_MEMORY(fileNameStrPtr))
				{
					const int fileMatch = strcmp(fileNameStrPtr, descriptor[i].fileName);
					if (0 == fileMatch)
					{
						const int nameMatch = strcmp(nameStrPtr, descriptor[i].variableName);
						if (0 == nameMatch)
						{
							/* found the correct entry */
							/* test if it matches the size of our requested data */
							if (curRecord->dataCnt == descriptor[i].dataLen)
							{
								if (IS_POINTER_IN_CONFIG_MEMORY(((const uint8_t*) curRecord) + curRecord->dataOffset))
								{
									configDataLoadMemory = ((const uint8_t*) curRecord) + curRecord->dataOffset;
								} else
								{
									/* error condition: use default values */
								}
								break;
							}
						}
					}
				}

				curRecord = (const configRecordEntry_t*)(((const uint8_t*)curRecord) + curRecord->nextEntryOffset);
			}
		}

		/* retreive the configuration values and put them into the desired space in the RAM */
		if (configDataLoadMemory != NULL)
		{
			memcpy(descriptor[i].dataPtr, configDataLoadMemory, descriptor[i].dataLen);
		} else
		{
			/* set to 0 as default */
			memset(descriptor[i].dataPtr, 0, descriptor[i].dataLen);
		}
	}
}

static void writeWordsToFlash(void *dst, const void *data, uint16_t size)
{
	size = size / 2; /* cnt of half word writes */
	const uint16_t *halfWords = (const uint16_t*) data;
	uint16_t *dstHalfWords = (uint16_t*) dst;
	uint16_t i = 0U;
	for (i = 0U; i < size; ++i)
	{
		flash_program_half_word((uint32_t)&(dstHalfWords[i]), halfWords[i], FLASH_PROGRAMM_ACCESS_SIZE);
	}
}

void config_updateToFlash()
{
	uint16_t *dst = &_flashConfigROMBegin;

	uint16_t i = 0U;

	const configVariableDescriptor_t *descriptor = (const configVariableDescriptor_t *)&_applicationConfigDescriptorsBegin;
	const uint16_t descriptorsCnt = ((uint32_t)&_applicationConfigDescriptorsEnd - (uint32_t)&_applicationConfigDescriptorsBegin) / sizeof(configVariableDescriptor_t);

	uint16_t descriptorsFoundCnt = 0U;

	/* test if there is need for an update */
	bool needsUpdate = false;

	for (i = 0U; (i < descriptorsCnt && false == needsUpdate); ++i)
	{
		const configRecordEntry_t *curRecord = (const configRecordEntry_t *)&_flashConfigROMBegin;

		while (NULL != curRecord &&
				IS_POINTER_IN_CONFIG_MEMORY(curRecord))
		{
			if ((0 == curRecord->dataCnt)         || ((uint32_t)~0 == curRecord->dataCnt)         ||
				(0 == curRecord->dataOffset)      || ((uint32_t)~0 == curRecord->dataOffset)      ||
				(0 == curRecord->filenameOffset)  || ((uint32_t)~0 == curRecord->filenameOffset)  ||
				(0 == curRecord->nameOffset)      || ((uint32_t)~0 == curRecord->nameOffset)      ||
				(0 == curRecord->nextEntryOffset) || ((uint32_t)~0 == curRecord->nextEntryOffset))
			{
				/* this is the guard keeper or just an invalid entry (marking the end of all records) */
				/* get out of the loop */
				break;
			} else
			{
				/* test if this is the matching record in the configuration */
				const char *nameStrPtr     = ((const char*) curRecord) + curRecord->nameOffset;
				const char *fileNameStrPtr = ((const char*) curRecord) + curRecord->filenameOffset;
				if (IS_POINTER_IN_CONFIG_MEMORY(nameStrPtr) && IS_POINTER_IN_CONFIG_MEMORY(fileNameStrPtr))
				{
					const int fileMatch = strcmp(fileNameStrPtr, descriptor[i].fileName);
					if (0 == fileMatch)
					{
						const int nameMatch = strcmp(nameStrPtr, descriptor[i].variableName);
						if (0 == nameMatch)
						{
							/* found the correct entry */
							/* test if it matches the size of our requested data */
							if (curRecord->dataCnt == descriptor[i].dataLen)
							{
								if (IS_POINTER_IN_CONFIG_MEMORY(((const uint8_t*) curRecord) + curRecord->dataOffset))
								{
									++descriptorsFoundCnt;
									/* test if the contents are the same
									 * if so there is no need to update this configuration entry
									 */
									const void *configFlashMemory = ((const uint8_t*) curRecord) + curRecord->dataOffset;
									const int memoryIntegrity = memcmp(descriptor[i].dataPtr, configFlashMemory, descriptor[i].dataLen);
									if (0 != memoryIntegrity)
									{
										/* the flash needs to be updated */
										needsUpdate = true;
									}
								} else
								{
									/* error condition the record entries are corrupt */
									/* the flash needs to be updated */
									needsUpdate = true;
								}
								break;
							}
						}
					}
				} else
				{
					needsUpdate = true;
					break;
				}
				curRecord = (const configRecordEntry_t*)(((const uint8_t*)curRecord) + curRecord->nextEntryOffset);
			}
		}
	}

	if (descriptorsCnt != descriptorsFoundCnt)
	{
		needsUpdate = true;
	}

	if (false != needsUpdate)
	{
		(void) flash_unlock();
		flash_erase_sector(FLASH_CONFIG_SECTOR, FLASH_PROGRAMM_ACCESS_SIZE);

		/* update the entire list of configuration records */
		for (i = 0U; i < descriptorsCnt; ++i)
		{
			uint16_t *dataStoragePtr;
			uint16_t *nameStoragePtr;
			uint16_t *fileNameStoragePtr;
			configRecordEntry_t curEntry;

			/* the string must have an even length */
			uint32_t nameStrLen = strlen(descriptor[i].variableName) + 1; /* + 1 for the trailing '\0'*/
			nameStrLen = (nameStrLen + 1) & ~1;

			uint32_t fileNameStrLen = strlen(descriptor[i].fileName) + 1; /* + 1 for the trailing '\0'*/
			fileNameStrLen = (fileNameStrLen + 1) & ~1;

			curEntry.dataOffset = sizeof(curEntry) + nameStrLen + fileNameStrLen;
			curEntry.nameOffset = sizeof(curEntry);
			curEntry.filenameOffset = sizeof(curEntry) + nameStrLen;
			curEntry.dataCnt    = descriptor[i].dataLen;

			uint32_t offset = curEntry.dataOffset + descriptor[i].dataLen;

			/* pad the next Entry for 4 byte alignment */
			if (offset & 0x3)
			{
				offset = (offset + 3) & (~3);
			}
			curEntry.nextEntryOffset = offset;

			/* write the header of the current entry */
			writeWordsToFlash(dst, &curEntry, sizeof(curEntry));

			dataStoragePtr = dst + curEntry.dataOffset / sizeof(*dst);

			nameStoragePtr = dst + curEntry.nameOffset / sizeof(*dst);
			fileNameStoragePtr = dst + curEntry.filenameOffset / sizeof(*dst);

			/* write the descriptors to the flash */
			writeWordsToFlash(nameStoragePtr, descriptor[i].variableName, nameStrLen);
			writeWordsToFlash(fileNameStoragePtr, descriptor[i].fileName, fileNameStrLen);

			/* write the config data to the flash */
			writeWordsToFlash(dataStoragePtr, descriptor[i].dataPtr, descriptor[i].dataLen);

			dst += curEntry.nextEntryOffset / sizeof(*dst);
		}

		/* as a stop token of the configuration data insert an empty configRecordEntry (EndOfConfiguration) */
		{
			configRecordEntry_t lastEntry;
			lastEntry.dataCnt = 0U;
			lastEntry.dataOffset = 0U;
			lastEntry.filenameOffset = 0U;
			lastEntry.nameOffset = 0U;
			lastEntry.nextEntryOffset = 0U;
			writeWordsToFlash(dst, &lastEntry, sizeof(lastEntry));
		}
		(void) flash_lock();
	}
}

static void configuration_init(void);
MODULE_INIT_FUNCTION(configuration, 3, configuration_init)
static void configuration_init(void)
{
	/* enable flash interface  clock. */
	 (void) readCurrentConfigurationFromFlash();
	 config_updateToFlash();

}

