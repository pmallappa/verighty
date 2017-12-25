SECTIONS
{
       .tests_section :
        {
                PROVIDE(__test_section_start = .);
                *(tests_section)
        }
        PROVIDE(__test_section_end = .);
}
