#include "../PluginStructs/P037_data_struct.h"

#ifdef USES_P037

#include "../Helpers/ESPEasy_Storage.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/WebServer.h"
#include "../WebServer/Markup.h"
#include "../WebServer/HTML_wrappers.h"
#include "../ESPEasyCore/ESPEasyRules.h"

P037_data_struct::P037_data_struct(taskIndex_t taskIndex) : _taskIndex(taskIndex) {
  loadSettings();
}

P037_data_struct::~P037_data_struct() {}

/**
 * Load the settings from file
 */
bool P037_data_struct::loadSettings() {
  if (_taskIndex < TASKS_MAX) {
    LoadCustomTaskSettings(_taskIndex, (byte*)&StoredSettings, sizeof(StoredSettings));
    return true;
  }
  return false;
}

#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
/**
 * Parse the mappings and filters from the settings-string into arrays
 */
void P037_data_struct::parseMappings() {

  if (
#ifdef P037_MAPPING_SUPPORT
    _maxIdx == -1
#endif
#if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
    ||
#endif
#ifdef P037_FILTER_SUPPORT
    _maxFilter == -1
#endif
    ) {
#ifdef P037_MAPPING_SUPPORT
    _maxIdx = 0; // Initialize to empty
#endif
#ifdef P037_FILTER_SUPPORT
    _maxFilter = 0; // Initialize to empty
#endif
    for (int8_t i = 0; i < P037_MAX_MAPPINGS; i++) {
#ifdef P037_MAPPING_SUPPORT
      _mapping[i] = F("");
#endif
#ifdef P037_FILTER_SUPPORT
      _filter[i] = F("");
#endif
    }

    String filterMap;
    String valueMap = String(StoredSettings.valueMappings);
    int16_t pipe = valueMap.indexOf(F("|"));
    if (pipe > -1) {
      filterMap = valueMap.substring(pipe + 1);
      valueMap = valueMap.substring(0, pipe);
    }
    #ifdef PLUGIN_037_DEBUG
    String debug;
    debug.reserve(64);
    #endif

    int16_t parse;
    int8_t operandIndex;
#ifdef P037_MAPPING_SUPPORT
    String operands = P037_OPERAND_LIST; // Anticipate more operations

    while (valueMap.length() > 0 && _maxIdx < P037_MAX_MAPPINGS * 3) {
      int16_t comma   = valueMap.indexOf(F(","));
      int16_t equals  = valueMap.indexOf(operands.substring(0, 1));
      int16_t percent = valueMap.indexOf(operands.substring(1, 2));
      if (comma == -1) comma = valueMap.length(); // last value
      if ((equals == -1 && percent > -1) || (equals > -1 && percent > -1 && percent < equals)) {
        operandIndex = 1;
        parse = percent;
      } else {
        operandIndex = 0;
        parse = equals;
      }
      _mapping[_maxIdx + 0] = valueMap.substring(0, parse);
      _mapping[_maxIdx + 1] = operands.substring(operandIndex, operandIndex + 1);
      _mapping[_maxIdx + 2] = valueMap.substring(parse + 1, comma);
      #ifdef PLUGIN_037_DEBUG
      if (debug.length() > 50) {
        addLog(LOG_LEVEL_DEBUG, debug);
        debug = F("");
      }
      if (debug.length() == 0) {
        debug = F("P037 mapping:");
      }
      debug += ' ';
      debug += _mapping[_maxIdx + 0];
      debug += ' ';
      debug += _mapping[_maxIdx + 1];
      debug += ' ';
      debug += _mapping[_maxIdx + 2];
      debug += ';';
      #endif
      valueMap = valueMap.substring(comma + 1);
      _maxIdx += 3;
      delay(0);
    }
    #ifdef PLUGIN_037_DEBUG
    if (debug.length() > 0) {
      addLog(LOG_LEVEL_DEBUG, debug);
      debug = F("");
    }
    #endif
#endif // P037_MAPPING_SUPPORT

#ifdef P037_FILTER_SUPPORT
    String filters = P037_FILTER_LIST; // Anticipate more filters
    while (filterMap.length() > 0 && _maxFilter < P037_MAX_MAPPINGS * 3) {
      int16_t comma   = filterMap.indexOf(F(","));
      int16_t equals  = filterMap.indexOf(filters.substring(0, 1));
      int16_t dash    = filterMap.indexOf(filters.substring(1, 2));
      if (comma == -1) comma = filterMap.length(); // last value
      if ((equals == -1 && dash > -1) || (equals > -1 && dash > -1 && dash < equals)) {
        operandIndex = 1;
        parse = dash;
      } else {
        operandIndex = 0;
        parse = equals;
      }
      _filter[_maxFilter + 0] = filterMap.substring(0, parse);
      _filter[_maxFilter + 1] = filters.substring(operandIndex, operandIndex + 1);
      _filter[_maxFilter + 2] = filterMap.substring(parse + 1, comma);
      #ifdef PLUGIN_037_DEBUG
      if (debug.length() > 50) {
        addLog(LOG_LEVEL_DEBUG, debug);
        debug = F("");
      }
      if (debug.length() == 0) {
        debug = F("P037 filter:");
      }
      debug += ' ';
      debug += _filter[_maxFilter + 0];
      debug += ' ';
      debug += _filter[_maxFilter + 1];
      debug += ' ';
      debug += _filter[_maxFilter + 2];
      debug += ';';
      #endif
      filterMap = filterMap.substring(comma + 1);
      _maxFilter += 3;
      delay(0);
    }
    #ifdef PLUGIN_037_DEBUG
    if (debug.length() > 0) {
      addLog(LOG_LEVEL_DEBUG, debug);
    }
    #endif
#endif // P037_FILTER_SUPPORT
  }
} // parseMappings
#endif // P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

bool P037_data_struct::webform_load(
#ifdef P037_MAPPING_SUPPORT
                                    bool mappingEnabled
#endif
#if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
                                    ,
#endif
#ifdef P037_FILTER_SUPPORT
                                    bool filterEnabled
#endif
#if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
                                    ,
#endif
#ifdef P037_JSON_SUPPORT
                                    bool jsonEnabled
#endif
                                   ) {

  bool success = false;

  addFormSubHeader(F("Topic subscriptions"));

#ifdef P037_JSON_SUPPORT
  if (jsonEnabled) {
      addRowLabel(F("MQTT Topic"), F(""));
      html_table(F(""), false);  // Sub-table
      html_table_header(F("&nbsp;#&nbsp;"));
      html_table_header(F("Topic"), 500);
      html_table_header(F("JSON Attribute"), 200);
  }
#endif
  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
#ifdef P037_JSON_SUPPORT
    if (jsonEnabled) { // Add a column with the json attribute to use for value
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtml(String(varNr + 1));
      html_TD();
      addTextBox(String(F("p037_template")) + (varNr + 1),
                StoredSettings.deviceTemplate[varNr],
                40,
                false, false, F(""), F("wide"));
      html_TD();
      addTextBox(String(F("p037_attribute")) + (varNr + 1),
                StoredSettings.jsonAttributes[varNr],
                20,
                false, false, F(""), F(""));
      html_TD();
    } else {
#endif
      addFormTextBox(String(F("MQTT Topic ")) + (varNr + 1), String(F("p037_template")) +
          (varNr + 1), StoredSettings.deviceTemplate[varNr], 40);
#ifdef P037_JSON_SUPPORT
    }
#endif
  }
#ifdef P037_JSON_SUPPORT
  if (jsonEnabled) {
    html_end_table();
  }
#endif

#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  parseMappings();
#endif

#ifdef P037_MAPPING_SUPPORT
  if (mappingEnabled) {
    addFormSubHeader(F("Name - value mappings"));
    addFormNote(F("Name - value mappings are case-sensitive. Do not use ',' or '|'."));

    addRowLabel(F("Mapping"), F(""));
    html_table(F(""), false);  // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    String operandOptions[P037_OPERAND_COUNT];
    operandOptions[0] = F("map");        // map name to int
    operandOptions[1] = F("percentage"); // map attribute value to percentage of provided value
    int operandIndices[P037_OPERAND_COUNT] = { 0, 1 };

    String operands = P037_OPERAND_LIST; // Anticipate more operations
    int8_t operandIndex;

    String info;
    info.reserve(25);

    int8_t idx;
    int8_t mapNr = 1;
    for (idx = 0; idx < _maxIdx; idx += 3) {

      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtml(String(mapNr));
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 0),
                _mapping[idx + 0],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      operandIndex = operands.indexOf(_mapping[idx + 1]);
      addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 2),
                _mapping[idx + 2],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      mapNr++;
    }
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 maxIdx: ");
    info += _maxIdx;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    operandIndex = 0;
    uint8_t extraMappings = 0;
    while (extraMappings < P037_EXTRA_VALUES && idx < P037_MAX_MAPPINGS * 3) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtml(String(mapNr));
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 0),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 2),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      idx += 3;
      extraMappings++;
      mapNr++;
    }
    html_end_table();
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 extraMappings: ");
    info += extraMappings;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    addFormNote(F("Both Name and Value must be filled for a valid mapping."));
    if (extraMappings == P037_EXTRA_VALUES) {
      String moreMessage = F("After filling all mappings, submitting this page will make extra mappings available (up to ");
      moreMessage += P037_MAX_MAPPINGS;
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
  }
#endif // P037_MAPPING_SUPPORT

#ifdef P037_FILTER_SUPPORT
  if (filterEnabled) {
    addFormSubHeader(F("Name - value filters"));
    addFormNote(F("Name - value filters are case-sensitive. Do not use ',' or '|'."));

    addRowLabel(F("Filter"), F(""));
    html_table(F(""), false);  // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name[;Index]"));
    //TODO: tonhuisman html_table_header(F("Name[;Index][#TopicId]"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    String filterOptions[P037_FILTER_COUNT];
    filterOptions[0] = F("equals");     // map name to value
    filterOptions[1] = F("range");      // between 2 values
    #if P037_FILTER_COUNT >= 3
    filterOptions[2] = F("list");       // list of multiple values
    #endif
    int filterIndices[P037_FILTER_COUNT] = { 0, 1
    #if P037_FILTER_COUNT >= 3
    , 2
    #endif
    };

    String filters = P037_FILTER_LIST; // Anticipate more filters
    int8_t filterIndex;

    String info;
    info.reserve(25);

    int8_t idx;
    int8_t filterNr = 1;
    for (idx = 0; idx < _maxFilter; idx += 3) {

      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtml(String(filterNr));
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 0),
                _filter[idx + 0],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      filterIndex = filters.indexOf(_filter[idx + 1]);
      addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 2),
                _filter[idx + 2],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      addUnit(F("Range/List: separate values with ; "));
      html_TD();
      filterNr++;
    }
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 maxFilter: ");
    info += _maxFilter;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    filterIndex = 0;
    uint8_t extraFilters = 0;
    while (extraFilters < P037_EXTRA_VALUES && idx < P037_MAX_FILTERS * 3) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtml(String(filterNr));
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 0),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 2),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      addUnit(F("Range/List: separate values with ; "));
      html_TD();
      idx += 3;
      extraFilters++;
      filterNr++;
    }
    html_end_table();
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 extraFilters: ");
    info += extraFilters;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    addFormNote(F("Both Name and Value must be filled for a valid filter."));
    if (extraFilters == P037_EXTRA_VALUES) {
      String moreMessage = F("After filling all filters, submitting this page will make extra filters available (up to ");
      moreMessage += P037_MAX_FILTERS;
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
  }
#endif  // P037_FILTER_SUPPORT

  success = true;
  return success;
} // webform_load

bool P037_data_struct::webform_save(
#ifdef P037_FILTER_SUPPORT
                                    bool filterEnabled
#endif
#if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
                                    ,
#endif
#ifdef P037_JSON_SUPPORT
                                    bool jsonEnabled
#endif
                                   ) {
  bool success = false;

  String error;
  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    String argName = F("p037_template");
    argName += varNr + 1;
    if (!safe_strncpy(StoredSettings.deviceTemplate[varNr], web_server.arg(argName).c_str(), sizeof(StoredSettings.deviceTemplate[varNr]))) {
      error += getCustomTaskSettingsError(varNr);
    }
#ifdef P037_JSON_SUPPORT
    if (jsonEnabled) {
      argName = F("p037_attribute");
      argName += varNr + 1;
      if (!safe_strncpy(StoredSettings.jsonAttributes[varNr], web_server.arg(argName).c_str(), sizeof(StoredSettings.jsonAttributes[varNr]))) {
        error += getCustomTaskSettingsError(varNr);
      }
    }
#endif // P037_JSON_SUPPORT
  }

#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  String valueMap;
  valueMap.reserve(sizeof(StoredSettings.valueMappings) / 2);

  String left, right;
  bool firstError = true;
#endif

#ifdef P037_MAPPING_SUPPORT
  String operands = P037_OPERAND_LIST;
  uint8_t mapNr = 1;
  left.reserve(32);
  right.reserve(32);
  for (int8_t idx = 0; idx < P037_MAX_MAPPINGS * 3; idx += 3) {
    left =  web_server.arg(getPluginCustomArgName(idx + 0));
    left.trim();
    right = web_server.arg(getPluginCustomArgName(idx + 2));
    right.trim();
    if (left.length() > 0 || right.length() > 0) {
      if (valueMap.length() > 0) {
        valueMap += ',';
      }
      valueMap += left;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 1));
      valueMap += operands.substring(oper, oper + 1);
      valueMap += right;
    }
    if ((left.length() == 0 && right.length() > 0) || (left.length() > 0 && right.length() == 0)) {
      if (firstError) {
        error += F("Name and value should both be filled for mapping ");
        firstError = false;
      } else {
        error += ',';
      }
      error += mapNr;
    }
    mapNr++;
    delay(0); // leave some yield
  }
  if (!firstError) {
    error += '\n';
  }
#endif

#ifdef P037_FILTER_SUPPORT
  String filters = P037_FILTER_LIST;
  firstError = true;
  String filterMap;
  uint8_t filterNr = 1;
  filterMap.reserve(sizeof(StoredSettings.valueMappings) / 2);
  for (int8_t idx = 0; idx < P037_MAX_FILTERS * 3; idx += 3) {
    left =  web_server.arg(getPluginCustomArgName(idx + 100 + 0));
    left.trim();
    right = web_server.arg(getPluginCustomArgName(idx + 100 + 2));
    right.trim();
    if (left.length() > 0 || right.length() > 0) {
      if (filterMap.length() > 0) {
        filterMap += ',';
      }
      filterMap += left;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 100 + 1));
      filterMap += filters.substring(oper, oper + 1);
      filterMap += right;
    }
    if ((left.length() == 0 && right.length() > 0) || (left.length() > 0 && right.length() == 0)) {
      if (firstError) {
        error += F("Name and value should both be filled for filter ");
        firstError = false;
      } else {
        error += ',';
      }
      error += filterNr;
    }
    filterNr++;
    delay(0); // leave some yield
  }
  if (!firstError) {
    error += '\n';
  }
  if (filterMap.length() > 0) { // Append filters to mappings if used
    valueMap += '|';
    valueMap += filterMap;
  }
#endif // P037_FILTER_SUPPORT

#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  if (!safe_strncpy(StoredSettings.valueMappings, valueMap.c_str(), sizeof(StoredSettings.valueMappings))) {
    error += F("Total combination of mappings/filters too long to store.\n");
  }
#endif

  if (error.length() > 0) {
    addHtmlError(error);
  }
#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  #ifdef PLUGIN_037_DEBUG
  String info = F("P037 Saved mappings/filters, length: ");
  info += valueMap.length();
  info += F(" free: ");
  info += sizeof(StoredSettings.valueMappings) - valueMap.length();
  addLog(LOG_LEVEL_INFO, info);
  addLog(LOG_LEVEL_INFO, valueMap);
  #endif
#endif
  SaveCustomTaskSettings(_taskIndex, (byte*)&StoredSettings, sizeof(StoredSettings));

#ifdef P037_MAPPING_SUPPORT
  _maxIdx = -1; // Invalidate current mappings and filters
#endif
#ifdef P037_FILTER_SUPPORT
  _maxFilter = -1;
#endif

  success = true;

  return success;
} // webform_save

#ifdef P037_MAPPING_SUPPORT
void P037_data_struct::logMapValue(String input, String result) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String info;
    info.reserve(45);
    info  = F("IMPT : MQTT 037 mapped value '");
    info += input;
    info += F("' to '");
    info += result;
    info += '\'';
    addLog(LOG_LEVEL_INFO, info);
  }
} // logMapValue

/**
 * Map a string to a (numeric) value, unchanged if no mapping found
 */
String P037_data_struct::mapValue(String input, String attribute) {
  String result = String(input); // clone
  if (input.length() > 0) {
    parseMappings();
    String operands = P037_OPERAND_LIST;
    for (int8_t idx = 0; idx < _maxIdx; idx += 3) {
      if (_mapping[idx + 0] == input || (attribute.length() > 0 && _mapping[idx + 0] == attribute)) {
        int8_t operandIndex = operands.indexOf(_mapping[idx + 1]);
        switch(operandIndex) {
          case 0: // = => 1:1 mapping
          {
            if (_mapping[idx + 2].length() > 0) {
              result = _mapping[idx + 2];
              logMapValue(input, result);
            }
            break;
          }
          case 1: // % => percentage of mapping
          {
            float inputFloat;
            float mappingFloat;
            if (string2float(input, inputFloat) && string2float(_mapping[idx + 2], mappingFloat)) {
              if (compareValues('>', mappingFloat, 0.0)) {
                float resultFloat = (100.0f / mappingFloat) * inputFloat; // Simple calculation to percentage
                int8_t decimals = 0;
                int8_t dotPos = input.indexOf('.');
                if (dotPos > -1) {
                  String decPart = input.substring(dotPos + 1);
                  decimals = decPart.length(); // Take the number of decimals to the output value
                }
                result = toString(resultFloat, decimals); // Percentage with same decimals as input
                logMapValue(input, result);
              }
            }
            break;
          }
          default:
          break;
        }
      }
    }
  }

  return result;
} // mapValue
#endif // P037_MAPPING_SUPPORT

#ifdef P037_FILTER_SUPPORT
/**
 * do we have filter values?
 */
bool P037_data_struct::hasFilters() {
  parseMappings(); // When not parsed yet
  return _maxFilter > 0;
} // hasFilters

#ifdef PLUGIN_037_DEBUG
void P037_data_struct::logFilterValue(String text, String key, String value, String match) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(50);
    log  = text;
    log += key;
    log += F(" value: ");
    log += value;
    log += F(" match: ");
    log += match;
    addLog(LOG_LEVEL_INFO, log);
  }
} // logFilterValue
#endif // PLUGIN_037_DEBUG

/**
 * checkFilters
 * Algorithm: (all comparisons are case-sensitive)
 * - If key is not found in the list, return true
 * - If key is found and value matches, return true
 * - if key is found but value doesn't match, return false
 * key can be in the list multiple times
 */
bool P037_data_struct::checkFilters(String key, String value, int8_t topicId) {
  bool result = true;

  if (key.length() > 0 && value.length() > 0) { // Ignore empty input(s)
    String filters = P037_FILTER_LIST;
    String valueData = value;
    String fltKey, fltIndex, filterData;
    float from, to, floatValue;
    int8_t rangeSeparator;
    bool accept, matchTopicId = true;

    for (uint8_t flt = 0; flt < _maxFilter; flt += 3) {
      fltKey = _filter[flt + 0];
      //TODO: tonhuisman Parse filter name[;index][#topicId] for optional TopicId to see if this filter is specific to a TopicID
      rangeSeparator = fltKey.indexOf('#');
      if (rangeSeparator > -1) {
        fltIndex = fltKey.substring(rangeSeparator + 1);
        fltKey = fltKey.substring(0, rangeSeparator); // Remove #topicId part
        // if (topicId > 0) {
        //   matchTopicId = (fltIndex.toInt() == topicId);
        //   // #ifdef PLUGIN_037_DEBUG
        //   // logFilterValue(F("P037 filter TopicId match on key: "), key, _filter[flt + 0], String(topicId) + (matchTopicId ? F(": true") : F(": false")));
        //   // #endif
        // }
      }
      // Parse filter name[;index] into name and index
      fltKey.replace(';', ',');
      fltIndex = parseString(fltKey, 2);
      rangeSeparator = fltIndex.toInt();
      if (rangeSeparator > 1) {
        valueData.replace(';', ',');
        valueData = parseString(valueData, rangeSeparator);
      }
      fltKey = parseString(fltKey, 1);
      fltKey.trim();
      if (fltKey == key) {
        result = false; // Matched key, so now we are looking for matching value
        int8_t filterIndex = filters.indexOf(_filter[flt + 1]);
        filterData = _filter[flt + 2];
        parseSystemVariables(filterData, false); // Replace system variables
        switch(filterIndex) {
          case 0: // = => equals
          {
            if (filterData == valueData) {
              #ifdef PLUGIN_037_DEBUG
              logFilterValue(F("P037 filter equals key: "), key, valueData, _filter[flt + 2] + (topicId > 0 ? (String(F(" topic match: ")) + String(matchTopicId ? F("yes"): F("no"))): String(F(""))));
              #endif
              return matchTopicId; // Match, don't look any further
            }
            break;
          }
          case 1: // - => range x-y (inside) or y-x (outside)
          {
            rangeSeparator = filterData.indexOf(';');
            if (rangeSeparator == -1) {
              rangeSeparator = filterData.indexOf('-'); // Fall-back test for dash
            }
            if (rangeSeparator > -1) {
              accept = false;;
              if (string2float(filterData.substring(0, rangeSeparator),  from) 
               && string2float(filterData.substring(rangeSeparator + 1), to)
               && string2float(valueData,                                floatValue)) {
                if (compareValues('>' + '=', to, from)) { // Normal low - high range: between low and high
                  if (compareValues('>' + '=', floatValue, from) && compareValues('<' + '=', floatValue, to)) {
                    accept = true;
                  }
                } else { // Alternative high - low range: outside low and high values
                  if (compareValues('>' + '=', floatValue, from) || compareValues('<' + '=', floatValue, to)) {
                    accept = true;
                  }
                }
                if (accept) {
                  #ifdef PLUGIN_037_DEBUG
                  logFilterValue(F("P037 filter in range key: "), key, valueData, _filter[flt + 2]);
                  #endif
                  return matchTopicId; // bail out, we're done
                #ifdef PLUGIN_037_DEBUG
                } else {
                  logFilterValue(F("P037 filter NOT in range key: "), key, valueData, _filter[flt + 2]);
                #endif
                }
              }
            }
            break;
          }
          #if P037_FILTER_COUNT >= 3
          case 2: // : => Match against a semicolon-separated list
          {
            String item;
            rangeSeparator = filterData.indexOf(';');
            if (rangeSeparator > -1 && string2float(valueData, floatValue)) {
              accept = false;
              do {
                item = filterData.substring(0, rangeSeparator);
                item.trim();
                filterData = filterData.substring(rangeSeparator + 1);
                filterData.trim();
                rangeSeparator = filterData.indexOf(';');
                if (rangeSeparator == -1) rangeSeparator = filterData.length(); // Last value
                if (string2float(item, from) && compareValues('=', floatValue, from)) {
                  accept = true;
                }
              } while (filterData.length() > 0 && !accept);
              if (accept) {
                #ifdef PLUGIN_037_DEBUG
                logFilterValue(F("P037 filter in list key: "), key, valueData, _filter[flt + 2]);
                #endif
                return matchTopicId; // bail out, we're done
              #ifdef PLUGIN_037_DEBUG
              } else {
                logFilterValue(F("P037 filter NOT in list key: "), key, valueData, _filter[flt + 2]);
              #endif
              }
            }            
            break;
          }
          #endif
          default:
          break;
        }
      }
      delay(0); // Allow some yield
    }
  }
  return result;
}
#endif  // P037_FILTER_SUPPORT

#endif  // ifdef USES_P037
