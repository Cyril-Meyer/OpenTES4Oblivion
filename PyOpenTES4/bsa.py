import warnings
import zlib
import utils
from utils import bytes_to_int


class BSA:
    def __init__(self, filename):
        self.filename = filename
        self.header = None
        self.folderRecords = None
        self.fileRecordBlocks = None
        self.fileNameBlock = None
        self.files = None

    def __repr__(self):
        return str(self.header) + "\n" +\
               str(self.folderRecords) + "\n" +\
               str(self.fileRecordBlocks)  + "\n" +\
               str(self.fileNameBlock)

    def read(self):
        # open file
        file = open(self.filename, 'rb')
        # read file header
        self.header = {
            "fileId": str(file.read(4)),
            "version": bytes_to_int(file.read(4)),
            "offset": bytes_to_int(file.read(4)),
            "archiveFlags": bytes_to_int(file.read(4)),
            "folderCount": bytes_to_int(file.read(4)),
            "fileCount": bytes_to_int(file.read(4)),
            "totalFolderNameLength": bytes_to_int(file.read(4)),
            "totalFileNameLength": bytes_to_int(file.read(4)),
            "fileFlags": bytes_to_int(file.read(4)),
        }
        # retrieve archive flags important flags
        archive_flags = self.header['archiveFlags']
        archive_flags_folder_name = (archive_flags & 0x1)
        archive_flags_file_name = (archive_flags & 0x2)
        archive_flags_file_compressed = (archive_flags & 0x4)
        # read folder records
        self.folderRecords = []
        for _ in range(self.header['folderCount']):
            folder_record = {
                "nameHash": bytes_to_int(file.read(8)),
                "count": bytes_to_int(file.read(4)),
                "offset": bytes_to_int(file.read(4)),
            }
            self.folderRecords.append(folder_record)
        # read file record blocs
        self.fileRecordBlocks = []
        for i in range(self.header['folderCount']):
            name = None
            if archive_flags_folder_name:
                name = utils.read_bzstring(file)
            # read file record
            file_records = []
            for _ in range(self.folderRecords[i]['count']):
                file_record = {
                    "nameHash": bytes_to_int(file.read(8)),
                    "fileSize": bytes_to_int(file.read(4)),
                    "offset": bytes_to_int(file.read(4)),
                }
                file_records.append(file_record)

            file_record_block = {
                "name": name,
                "fileRecord": file_records,
            }
            self.fileRecordBlocks.append(file_record_block)
        # read file name block
        self.fileNameBlock = []
        if archive_flags_file_name:
            for _ in range(self.header['fileCount']):
                self.fileNameBlock.append(utils.read_zstring(file))
        # read data
        self.files = []
        if archive_flags_file_compressed:
            # read compressed data
            for i in self.fileRecordBlocks:
                for j in i['fileRecord']:
                    # check offset
                    assert file.tell() == j['offset'], \
                        f'current position {file.tell()} in file and offset {j["offset"]} are not equal'
                    # read compressed file
                    uncompressed_size = bytes_to_int(file.read(4))
                    compressed_file = file.read(j['fileSize']-4)
                    # decompress file
                    z_obj = zlib.decompressobj()
                    uncompressed_file = z_obj.decompress(compressed_file)
                    # check decompressed size
                    assert uncompressed_size == len(uncompressed_file), \
                        f'decompression error, {uncompressed_size} not equal to {len(uncompressed_file)}'
                    self.files.append(uncompressed_file)
        else:
            # read uncompressed data
            for i in self.fileRecordBlocks:
                for j in i['fileRecord']:
                    self.files.append(file.read(j['fileSize']))

        byte = file.read(1)
        if byte:
            warnings.warn(f'Enf of read before end of file - {self.filename}')

        return 0
