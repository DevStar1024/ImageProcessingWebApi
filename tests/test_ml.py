import pytest
import os
import asyncio

class TestML:

    INPUT_FILE_PATH = '/app/mainApi/tests/test_image_tiles/aaa.tif'
    OUTPUT_FILE_PATH = '/app/mainApi/tests/ml_out/aaa.jpg'
    TEST_OUTPUT_FOLDER = '/app/mainApi/tests/ml_out'

    def setup_class(self):
        if not os.path.exists(self.TEST_OUTPUT_FOLDER):
            os.makedirs(self.TEST_OUTPUT_FOLDER)

    @pytest.mark.asyncio
    async def test_basic(self):
        cmd_str = "/app/mainApi/ml_lib/segA {inputPath} {outputPath} /app/mainApi/ml_lib/src_paramA.txt".format(
            inputPath=self.INPUT_FILE_PATH, outputPath=self.OUTPUT_FILE_PATH
        )
        # subprocess.call(cmd_str, shell=True)
        await asyncio.create_subprocess_shell(cmd_str)








