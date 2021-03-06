#include "Thermistor.hh"
#include <avr/eeprom.h>

Thermistor::Thermistor(Pin& pin, uint16_t table_offset) :
	pin_(pin), next_sample_(0), table_offset_(table_offset) {
	for (int i = 0; i < SAMPLE_COUNT; i++) { sample_buffer_[i] = 0; }
	pin_.setDirection(false);
}

void Thermistor::update() {
	sample_buffer_[i] = pin_.getAnalogValue();
	next_sample_ = (next_sample_+1) % SAMPLE_COUNT;

	// average
	int16_t cumulative = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		cumulative += sample_buffer_[i];
	}
	int16_t avg = cumulative / SAMPLE_COUNT;

	// lookup.  The tables are now guaranteed to have their entries at fixed intervals over the range of the ADC.
	const static int SLOT_WIDTH = ADC_RANGE/(THERM_TABLE_SIZE-1);

	int floor_table_index = (avg * (THERM_TABLE_SIZE-1)) / ADC_RANGE;
	int ceiling_table_index = floor_table_index+1;

	int entry_floor = (table_index * ADC_RANGE) / (THERM_TABLE_SIZE-1);

	int16_t floor = eeprom_read_word(table_offset + (4 * floor_table_index) + 2);
	int16_t ceiling = eeprom_read_word(table_offset + (4 * ceiling_table_index) + 2);

	current_temp_ =
			floor + ((avg - entry_floor) * (ceiling-floor))/SLOT_WIDTH;
}
