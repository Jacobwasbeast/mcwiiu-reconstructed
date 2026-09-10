#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP

namespace nn { namespace pia { namespace inet {

class InetProcessHostMigrationJob
{
public:
    void HostChangeEventNotice(unsigned int sessionId);

    unsigned char m_padding[0x1c9];
    bool m_hostChanged;
};

}}}

#endif
