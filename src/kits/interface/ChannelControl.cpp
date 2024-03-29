/* 
 * Copyright 2005-2014 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#include <ChannelControl.h>
#include <PropertyInfo.h>

#include <map>
#include <kits/support/String>

struct limit_label {
	std::string min_label;
	std::string max_label;
};

typedef std::map<int32_t, limit_label> label_map;

static property_info
sPropertyInfo[] = {
	{ "ChannelCount",
		{ B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		{ B_DIRECT_SPECIFIER, 0 }, NULL, 0, { B_INT32_TYPE }
	},

	{ "CurrentChannel",
		{ B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		{ B_DIRECT_SPECIFIER, 0 }, NULL, 0, { B_INT32_TYPE }
	},

	{ "MaxLimitLabel", 
		{ B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		{ B_DIRECT_SPECIFIER, 0 }, NULL, 0, { B_STRING_TYPE }
	},

	{ "MinLimitLabel",
		{ B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		{ B_DIRECT_SPECIFIER, 0 }, NULL, 0, { B_STRING_TYPE }
	},

	{ 0 }
};


BChannelControl::BChannelControl(BRect frame, const char* name,
	const char* label, BMessage* model, int32_t channel_count,
	uint32_t resizingMode, uint32_t flags)
	:
	BControl(frame, name, label, model, resizingMode, flags),
	fChannelCount(channel_count),
	fCurrentChannel(0),
	fChannelMin(NULL),
	fChannelMax(NULL),
	fChannelValues(NULL),
	fMultiLabels(NULL),
	fModificationMsg(NULL)
{
	fChannelMin = new int32_t[channel_count];
	memset(fChannelMin, 0, sizeof(int32_t) * channel_count);

	fChannelMax = new int32_t[channel_count];
	for (int32_t i = 0; i < channel_count; i++)
		fChannelMax[i] = 100;

	fChannelValues = new int32_t[channel_count];
	memset(fChannelValues, 0, sizeof(int32_t) * channel_count);

	fMultiLabels = (void*)new label_map;
}


BChannelControl::BChannelControl(const char* name, const char* label,
	BMessage* model, int32_t channelCount, uint32_t flags)
	:
	BControl(name, label, model, flags),
	fChannelCount(channelCount),
	fCurrentChannel(0),
	fChannelMin(NULL),
	fChannelMax(NULL),
	fChannelValues(NULL),
	fMultiLabels(NULL),
	fModificationMsg(NULL)
{
	fChannelMin = new int32_t[channelCount];
	memset(fChannelMin, 0, sizeof(int32_t) * channelCount);
	
	fChannelMax = new int32_t[channelCount];
	for (int32_t i = 0; i < channelCount; i++)
		fChannelMax[i] = 100;

	fChannelValues = new int32_t[channelCount];
	memset(fChannelValues, 0, sizeof(int32_t) * channelCount);

	fMultiLabels = (void*)new label_map;
}


BChannelControl::BChannelControl(BMessage* archive)
	:
	BControl(archive),
	fChannelCount(0),
	fCurrentChannel(0),
	fChannelMin(NULL),
	fChannelMax(NULL),
	fChannelValues(NULL),
	fMultiLabels(NULL),
	fModificationMsg(NULL)
{
	archive->FindInt32("be:_m_channel_count", &fChannelCount);
	archive->FindInt32("be:_m_value_channel", &fCurrentChannel);

	if (fChannelCount > 0) {
		fChannelMin = new int32_t[fChannelCount];
		memset(fChannelMin, 0, sizeof(int32_t) * fChannelCount);

		fChannelMax = new int32_t[fChannelCount];
		for (int32_t i = 0; i < fChannelCount; i++)
			fChannelMax[i] = 100;

		fChannelValues = new int32_t[fChannelCount];
		memset(fChannelValues, 0, sizeof(int32_t) * fChannelCount);

		for (int32_t c = 0; c < fChannelCount; c++) {
			archive->FindInt32("be:_m_channel_min", c, &fChannelMin[c]);
			archive->FindInt32("be:_m_channel_max", c, &fChannelMax[c]);
			archive->FindInt32("be:_m_channel_val", c, &fChannelValues[c]);
		}
	}

	const char* label = NULL;
	if (archive->FindString("be:_m_min_label", &label) == B_OK)
		fMinLabel = label;

	if (archive->FindString("be:_m_max_label", &label) == B_OK)
		fMaxLabel = label;

	BMessage* modificationMessage = new BMessage;
	if (archive->FindMessage("_mod_msg", modificationMessage) == B_OK)
		fModificationMsg = modificationMessage;
	else
		delete modificationMessage;

	fMultiLabels = (void*)new label_map;
}


BChannelControl::~BChannelControl()
{
	delete[] fChannelMin;
	delete[] fChannelMax;
	delete[] fChannelValues;
	delete fModificationMsg;
	delete reinterpret_cast<label_map*>(fMultiLabels);
}


status_t BChannelControl::Archive(BMessage* data, bool deep) const
{
	status_t status = BControl::Archive(data, deep);
	if (status == B_OK)
		status = data->AddInt32("be:_m_channel_count", fChannelCount);

	if (status == B_OK)
		status = data->AddInt32("be:_m_value_channel", fCurrentChannel);

	if (status == B_OK)
		status = data->AddString("be:_m_min_label", fMinLabel.String());

	if (status == B_OK)
		status = data->AddString("be:_m_max_label", fMaxLabel.String());

	if (status == B_OK && fChannelValues != NULL
		&& fChannelMax != NULL && fChannelMin != NULL) {
		for (int32_t i = 0; i < fChannelCount; i++) {
			status = data->AddInt32("be:_m_channel_min", fChannelMin[i]);
			if (status < B_OK)
				break;

			status = data->AddInt32("be:_m_channel_max", fChannelMax[i]);
			if (status < B_OK)
				break;

			status = data->AddInt32("be:_m_channel_val", fChannelValues[i]);
			if (status < B_OK)
				break;
		}
	}

	return status;
}


void BChannelControl::FrameResized(float newWidth, float newHeight)
{
	BView::FrameResized(newWidth, newHeight);
}


void BChannelControl::SetFont(const BFont* font, uint32_t mask)
{
	BView::SetFont(font, mask);
}


void BChannelControl::AttachedToWindow()
{
	BControl::AttachedToWindow();
}


void BChannelControl::DetachedFromWindow()
{
	BControl::DetachedFromWindow();
}


void BChannelControl::ResizeToPreferred()
{
	BControl::ResizeToPreferred();
}


void BChannelControl::MessageReceived(BMessage* message)
{
	BControl::MessageReceived(message);
}


BHandler*
BChannelControl::ResolveSpecifier(BMessage* message, int32_t index,
	BMessage* specifier, int32_t what, const char* property)
{
	BHandler* target = this;
	BPropertyInfo propertyInfo(sPropertyInfo);
	if (propertyInfo.FindMatch(message, index, specifier, what, property)
			< B_OK) {
		target = BControl::ResolveSpecifier(message, index, specifier,
			what, property);
	}

	return target;
}


status_t BChannelControl::GetSupportedSuites(BMessage* data)
{
	if (data == NULL)
		return B_BAD_VALUE;

	status_t err = data->AddString("suites", "suite/vnd.Be-channel-control");

	BPropertyInfo propertyInfo(sPropertyInfo);
	if (err == B_OK) 
		err = data->AddFlat("messages", &propertyInfo);

	if (err == B_OK)
		return BControl::GetSupportedSuites(data);

	return err;
}


void BChannelControl::SetModificationMessage(BMessage* message)
{
	delete fModificationMsg;
	fModificationMsg = message;
}


BMessage*  BChannelControl::ModificationMessage() const
{
	return fModificationMsg;
}


status_t BChannelControl::Invoke(BMessage* message)
{
	bool notify = false;
	BMessage invokeMessage(InvokeKind(&notify));

	if (message != NULL)
		invokeMessage = *message;
	else if (Message() != NULL)
		invokeMessage = *Message();
	
	invokeMessage.AddInt32("be:current_channel", fCurrentChannel);

	return BControl::Invoke(&invokeMessage);
}


status_t BChannelControl::InvokeChannel(BMessage* message, int32_t fromChannel,
	int32_t channelCount, const bool* _mask)
{
	bool notify = false;
	BMessage invokeMessage(InvokeKind(&notify));

	if (message != NULL)
		invokeMessage = *message;
	else if (Message() != NULL)
		invokeMessage = *Message();

	invokeMessage.AddInt32("be:current_channel", fCurrentChannel);
	if (channelCount < 0)
		channelCount = fChannelCount - fromChannel;

	for (int32_t i = 0; i < channelCount; i++) {
		invokeMessage.AddInt32("be:channel_value",
			fChannelValues[fromChannel + i]);
		invokeMessage.AddBool("be:channel_changed", _mask ? _mask[i] : true);
	}

	return BControl::Invoke(&invokeMessage);
}


status_t BChannelControl::InvokeNotifyChannel(BMessage* message, uint32_t kind,
	int32_t fromChannel, int32_t channelCount, const bool* _mask)
{
	BeginInvokeNotify(kind);
	status_t status = InvokeChannel(message, fromChannel, channelCount, _mask);
	EndInvokeNotify();

	return status;
}


void BChannelControl::SetValue(int32_t value)
{
	// Get real
	if (value > fChannelMax[fCurrentChannel])
		value = fChannelMax[fCurrentChannel];

	if (value < fChannelMin[fCurrentChannel])
		value = fChannelMin[fCurrentChannel];

	if (value != fChannelValues[fCurrentChannel]) {
		StuffValues(fCurrentChannel, 1, &value);
		BControl::SetValue(value);
	}
}


status_t BChannelControl::SetCurrentChannel(int32_t channel)
{
	if (channel < 0 || channel >= fChannelCount)
		return B_BAD_INDEX;

	if (channel != fCurrentChannel) {
		fCurrentChannel = channel;
		BControl::SetValue(fChannelValues[fCurrentChannel]);
	}

	return B_OK;	
}


int32_t BChannelControl::CurrentChannel() const
{
	return fCurrentChannel;
}


int32_t BChannelControl::CountChannels() const
{
	return fChannelCount;
}


status_t BChannelControl::SetChannelCount(int32_t channel_count)
{
	if (channel_count < 0 || channel_count >= MaxChannelCount())
		return B_BAD_VALUE;

	// TODO: Currently we only grow the buffer. Test what BeOS does
	if (channel_count > fChannelCount) {
		int32_t* newMin = new int32_t[channel_count];
		int32_t* newMax = new int32_t[channel_count];
		int32_t* newVal = new int32_t[channel_count];

		memcpy(newMin, fChannelMin, fChannelCount);
		memcpy(newMax, fChannelMax, fChannelCount);
		memcpy(newVal, fChannelValues, fChannelCount);

		delete[] fChannelMin;
		delete[] fChannelMax;
		delete[] fChannelValues;

		fChannelMin = newMin;
		fChannelMax = newMax;
		fChannelValues = newVal;
	}

	fChannelCount = channel_count;

	return B_OK;
}


int32_t BChannelControl::ValueFor(int32_t channel) const
{
	int32_t value = 0;
	if (GetValue(&value, channel, 1) <= 0)
		return -1;

	return value;
}


int32_t BChannelControl::GetValue(int32_t* outValues, int32_t fromChannel,
	int32_t channelCount) const
{
	int32_t i = 0;
	for (i = 0; i < channelCount; i++)
		outValues[i] = fChannelValues[fromChannel + i];

	return i;
}


status_t BChannelControl::SetValueFor(int32_t channel, int32_t value)
{
	return SetValue(channel, 1, &value);
}


status_t BChannelControl::SetValue(int32_t fromChannel, int32_t channelCount,
	const int32_t* values)
{
	return StuffValues(fromChannel, channelCount, values);
}


status_t BChannelControl::SetAllValue(int32_t values)
{
	int32_t* newValues = new int32_t[fChannelCount];
	for (int32_t i = 0; i < fChannelCount; i++) {
		int32_t limitedValue = max_c(values, MinLimitList()[i]);
		limitedValue = min_c(limitedValue, MaxLimitList()[i]);

		newValues[i] = limitedValue;
	}

	delete[] fChannelValues;
	fChannelValues = newValues;
	BControl::SetValue(fChannelValues[fCurrentChannel]);

	return B_OK;
}


status_t BChannelControl::SetLimitsFor(int32_t channel, int32_t minimum, int32_t maximum)
{
	return SetLimitsFor(channel, 1, &minimum, &maximum);
}


status_t BChannelControl::GetLimitsFor(int32_t channel, int32_t* minimum,
	int32_t* maximum) const
{
	return GetLimitsFor(channel, 1, minimum, maximum);
}


status_t BChannelControl::SetLimitsFor(int32_t fromChannel, int32_t channelCount,
	const int32_t* minimum, const int32_t* maximum)
{
	if (fromChannel + channelCount > CountChannels())
		channelCount = CountChannels() - fromChannel;

	for (int i = 0; i < channelCount; i++) {
		if (minimum[i] > maximum[i])
			return B_BAD_VALUE;

		fChannelMin[fromChannel + i] = minimum[i];
		fChannelMax[fromChannel + i] = maximum[i];
		if (fChannelValues[fromChannel + i] < minimum[i])
			fChannelValues[fromChannel + i] = minimum[i];
		else if (fChannelValues[fromChannel + i] > maximum[i])
			fChannelValues[fromChannel + i] = maximum[i];
	}

	return B_OK;
}


status_t BChannelControl::GetLimitsFor(int32_t fromChannel, int32_t channelCount,
	int32_t* minimum, int32_t* maximum) const
{
	if (minimum == NULL || maximum == NULL)
		return B_BAD_VALUE;

	if (fChannelMin == NULL || fChannelMax == NULL)
		return B_ERROR;
	if (fromChannel + channelCount > CountChannels())
		channelCount = CountChannels() - fromChannel;

	for (int i = 0; i < channelCount; i++) {
		minimum[i] = fChannelMin[fromChannel + i];
		maximum[i] = fChannelMax[fromChannel + i];
	}

	return B_OK;
}


status_t BChannelControl::SetLimits(int32_t minimum, int32_t maximum)
{
	if (minimum > maximum)
		return B_BAD_VALUE;

	int32_t numChannels = CountChannels();

	for (int32_t c = 0; c < numChannels; c++) {
		fChannelMin[c] = minimum;
		fChannelMax[c] = maximum;
		if (fChannelValues[c] < minimum)
			fChannelValues[c] = minimum;
		else if (fChannelValues[c] > maximum)
			fChannelValues[c] = maximum;
	}

	return B_OK;
}


status_t BChannelControl::GetLimits(int32_t* outMinimum, int32_t* outMaximum) const
{
	if (outMinimum == NULL || outMaximum == NULL)
		return B_BAD_VALUE;

	if (fChannelMin == NULL || fChannelMax == NULL)
		return B_ERROR;

	int32_t numChannels = CountChannels();
	for (int32_t c = 0; c < numChannels; c++) {
		outMinimum[c] = fChannelMin[c];
		outMaximum[c] = fChannelMax[c];
	}

	return B_OK;
}


status_t BChannelControl::SetLimitLabels(const char* minLabel, const char* maxLabel)
{
	if (minLabel != fMinLabel)
		fMinLabel = minLabel;

	if (maxLabel != fMaxLabel)
		fMaxLabel = maxLabel;

	Invalidate();

	return B_OK;
}


const char*  BChannelControl::MinLimitLabel() const
{
	return fMinLabel.String();
}


const char*  BChannelControl::MaxLimitLabel() const
{
	return fMaxLabel.String();
}


status_t BChannelControl::SetLimitLabelsFor(int32_t channel, const char* minLabel,
	const char* maxLabel)
{
	(*(label_map*)fMultiLabels)[channel].max_label = maxLabel;
	(*(label_map*)fMultiLabels)[channel].min_label = minLabel;
	return B_OK;
}


status_t BChannelControl::SetLimitLabelsFor(int32_t fromChannel, int32_t channelCount,
	const char* minLabel, const char* maxLabel)
{
	for (int32_t i = fromChannel; i < fromChannel + channelCount; i++) {
		SetLimitLabelsFor(i, minLabel, maxLabel);
	}
	return B_OK;
}


const char*  BChannelControl::MinLimitLabelFor(int32_t channel) const
{
	if (fMultiLabels != NULL) {
		label_map::const_iterator iter = ((label_map*)fMultiLabels)->find(channel);
		if (iter != ((label_map*)fMultiLabels)->end())
			return (*iter).second.min_label.c_str();
	}
	return NULL;
}


const char*  BChannelControl::MaxLimitLabelFor(int32_t channel) const
{
	if (fMultiLabels != NULL) {
		label_map::const_iterator iter = ((label_map*)fMultiLabels)->find(channel);
		if (iter != ((label_map*)fMultiLabels)->end())
			return (*iter).second.max_label.c_str();
	}
	return NULL;
}


status_t BChannelControl::StuffValues(int32_t fromChannel, int32_t channelCount,
	const int32_t* values)
{
	if (values == NULL)
		return B_BAD_VALUE;

	if (fromChannel < 0 || fromChannel > fChannelCount
		|| fromChannel + channelCount > fChannelCount) {
		return B_BAD_INDEX;
	}

	for (int32_t i = 0; i < channelCount; i++) {
		if (values[i] <= fChannelMax[fromChannel + i]
			&& values[i] >= fChannelMin[fromChannel + i]) {
			fChannelValues[fromChannel + i] = values[i];
		}
	}

	// if the current channel was updated, update also the control value
	if (fCurrentChannel >= fromChannel
		&& fCurrentChannel <= fromChannel + channelCount) {
		BControl::SetValue(fChannelValues[fCurrentChannel]);
	}

	return B_OK;
}


void BChannelControl::_Reserverd_ChannelControl_0(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_1(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_2(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_3(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_4(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_5(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_6(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_7(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_8(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_9(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_10(void*, ...) {}
void BChannelControl::_Reserverd_ChannelControl_11(void*, ...) {}
