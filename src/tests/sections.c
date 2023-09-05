#include <bfd.h>

bfd *abfd;
asection *p;
char *filename = "/path/to/my/file";

if ((abfd = bfd_openr(filename, NULL)) == NULL) {
    /* ... error handling */
}

if (!bfd_check_format (abfd, bfd_object)) {
    /* ... error handling */
}

for (p = abfd->sections; p != NULL; p = p->next) {
    bfd_vma  base_addr = bfd_section_vma(abfd, p);
    bfd_size_type size = bfd_section_size (abfd, p);
    const char   *name = bfd_section_name(abfd, p);
    flagword     flags = bfd_get_section_flags(abfd, p);

    if (flags & SEC_CODE) {
        printf("%s: addr=%p size=%d\n", name, base_addr, size);
    }
}

