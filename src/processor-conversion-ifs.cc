// This file was autogenerated by generate-conversion-ifs.rb; DO NOT EDIT

if( input_interleaveness == format_interleaved )
{
    if( output_interleaveness == format_interleaved )
    {
        if( input_representation == format_short )
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
        else
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, true, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, true, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
    }
    else
    {
        if( input_representation == format_short )
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
        else
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< true, false, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< true, false, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
    }
}
else
{
    if( output_interleaveness == format_interleaved )
    {
        if( input_representation == format_short )
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
        else
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, true, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, true, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
    }
    else
    {
        if( input_representation == format_short )
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, true, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, false, short, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, true, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, false, short, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
        else
        {
            if( output_representation == format_short )
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, true, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, false, float, short >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
            else
            {
                if( input_endianness == format_be )
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
                else
                {
                    if( output_endianness == format_be )
                    {
                        convert_template< false, false, true, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                    else
                    {
                        convert_template< false, false, false, float, float >( input, buffer_length(), &output, output_length, output_allocated, channels );
                    }
                }
            }
        }
    }
}
