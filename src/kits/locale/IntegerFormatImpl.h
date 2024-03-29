#ifndef _B_INTEGER_FORMAT_IMPL_H_
#define _B_INTEGER_FORMAT_IMPL_H_

struct format_field_position;
class BIntegerFormatParameters;
class BString;

class BIntegerFormatImpl {
	public:
		BIntegerFormatImpl();
		virtual ~BIntegerFormatImpl();

		// formatting

		virtual status_t Format(const BIntegerFormatParameters *parameters,
								int64 number, BString *buffer) const = 0;
		virtual status_t Format(const BIntegerFormatParameters *parameters,
								int64 number, BString *buffer,
								format_field_position *positions,
								int32_t positionCount = 1,
								int32_t *fieldCount = NULL,
								bool allFieldPositions = false) const = 0;

		// TODO: ...

		virtual BNumberFormatParameters *DefaultNumberFormatParameters();
		virtual const BNumberFormatParameters *DefaultNumberFormatParameters()
			const;

		virtual BIntegerFormatParameters *DefaultIntegerFormatParameters() = 0;
		virtual const BIntegerFormatParameters *DefaultIntegerFormatParameters()
			const = 0;
};


#endif	// _B_INTEGER_FORMAT_IMPL_H_
