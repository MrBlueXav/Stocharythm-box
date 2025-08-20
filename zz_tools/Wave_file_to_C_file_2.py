####################################################################################
#       WF2CF : convertit un lot de fichiers wav en un seul fichier C et son entête
#
# Les noms de fichiers wav doivent commencer par une lettre (et être un nom de variable valide en C)
# kick808.wav -> int16_t kick808[] = {....}
# Une banque de son est créée (un tableau de structs) : const struct Sample sampleBank[]
# Le nombre de samples est stocké dans : const unsigned int sampleBankCount
#
# Exemple d'utilisation :
# wav_to_c_array("dossier_de_mes_wavs", "wave_data.c", "wave_data.h")
#                   Î_ nom du dossier contenant les wav à traiter
####################################################################################

import os
import struct

def parse_wav_header(data):
    """Extrait les infos principales du header WAV"""
    if data[0:4] != b"RIFF" or data[8:12] != b"WAVE":
        return None
    
    # Bloc fmt
    audio_format, num_channels, sample_rate, byte_rate, block_align, bits_per_sample = struct.unpack("<HHIIHH", data[20:36])
    # Bloc data
    data_size = struct.unpack("<I", data[40:44])[0]

    return {
        "format": audio_format,
        "channels": num_channels,
        "sample_rate": sample_rate,
        "bits": bits_per_sample,
        "data_size": data_size
    }


def wav_to_c_array(input_path, output_c, output_h):
    files = [f for f in os.listdir(input_path) if f.lower().endswith(".wav")]

    with open(output_c, "w") as fc, open(output_h, "w") as fh:
        fc.write('#include "{}"\n'.format(os.path.basename(output_h)))
        fc.write('#include <stdint.h>\n\n')
        fh.write("#pragma once\n\n")
        fh.write("#include <stdint.h>\n\n")

        # Struct pour décrire un sample
        fh.write("struct Sample {\n")
        fh.write("    const void* data;\n")
        fh.write("    unsigned int length;\n")
        fh.write("    unsigned int sample_rate;\n")
        fh.write("    unsigned int channels;\n")
        fh.write("    unsigned int bits;\n")
        fh.write("};\n\n")

        sample_entries = []  # pour construire sampleBank

        for fname in files:
            path = os.path.join(input_path, fname)
            rawname = os.path.splitext(fname)[0].replace("-", "_").replace(" ", "_")
            #varname = "snd_" + rawname
            varname = rawname

            with open(path, "rb") as f:
                data = f.read()

            header = parse_wav_header(data)
            if not header:
                print(f"[!] {fname} n'est pas un fichier WAV valide.")
                continue

            pcm_data = data[44:]  # ignorer l'entête WAV

            # Déterminer le type C
            if header["bits"] == 8:
                c_type = "uint8_t"
                fmt = "B"
                step = 1
            elif header["bits"] == 16:
                c_type = "int16_t"
                fmt = "<h"
                step = 2
            elif header["bits"] == 32:
                c_type = "int32_t"
                fmt = "<i"
                step = 4
            else:
                print(f"[!] Profondeur {header['bits']} bits non supportée ({fname})")
                continue

            # --- .h ---
            fh.write(f"extern const {c_type} {varname}[];\n")
            fh.write(f"extern const unsigned int {varname}_len;\n\n")

            # --- .c ---
            fc.write(f"// {fname}\n")
            fc.write(f"// Format: {header['channels']} canaux, {header['sample_rate']} Hz, {header['bits']} bits\n")
            fc.write(f"// Taille PCM: {header['data_size']} octets\n\n")

            fc.write(f"const {c_type} {varname}[] = {{\n")

            values = []
            for i in range(0, len(pcm_data), step):
                val = struct.unpack(fmt, pcm_data[i:i+step])[0]
                values.append(val)

            for i, val in enumerate(values):
                if i % 10 == 0:
                    fc.write("    ")
                fc.write(f"{val}, ")
                if i % 10 == 9:
                    fc.write("\n")
            if len(values) % 10 != 0:
                fc.write("\n")

            fc.write("};\n")
            fc.write(f"const unsigned int {varname}_len = {len(values)};\n\n")

            # garder trace pour construire sampleBank
            sample_entries.append((varname, header))

        # Générer sampleBank
        fc.write("// Banque de samples\n")
        fc.write("const struct Sample sampleBank[] = {\n")
        for varname, header in sample_entries:
            fc.write(f"    {{ {varname}, {varname}_len, {header['sample_rate']}, {header['channels']}, {header['bits']} }},\n")
        fc.write("};\n\n")
        fc.write("const unsigned int sampleBankCount = sizeof(sampleBank) / sizeof(sampleBank[0]);\n")

        # Déclarations externes dans le .h
        fh.write("extern const struct Sample sampleBank[];\n")
        fh.write("extern const unsigned int sampleBankCount;\n")

    print(f"Conversion terminée : {len(files)} fichiers traités.")


# Exemple d’utilisation :
# wav_to_c_array("mes_wavs", "wave_data.c", "wave_data.h")
