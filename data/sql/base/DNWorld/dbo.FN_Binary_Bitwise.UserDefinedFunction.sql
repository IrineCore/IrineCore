USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_Binary_Bitwise]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE FUNCTION [dbo].[FN_Binary_Bitwise](@mask1 VARBINARY(MAX), @mask2 VARBINARY(MAX), @operator CHAR(1))
RETURNS VARBINARY(MAX)
AS
BEGIN
-- Set variable for slice size, INT = 4 bytes. Technically INT is signed, but since we're doing binary operations it won't matter.
DECLARE @slice_bytes INT;
SET @slice_bytes = 4;


DECLARE @max_length INT;                        -- Maximum length of either binary variable
DECLARE @slices INT;                            -- Number of slices
DECLARE @bin1 INT;                              -- Slice holder for @mask1
DECLARE @bin2 INT;                              -- Slice holder for @mask2
DECLARE @result INT;                            -- Result of bitwise binary operation per slice
DECLARE @resultmask VARBINARY(MAX);             -- Result of bitwise binary operation total
SET @resultmask = NULL;

-- Get maximum variable length
IF(DATALENGTH(@mask1) > DATALENGTH(@mask2))
SET @max_length = DATALENGTH(@mask1);
else
SET @max_length = DATALENGTH(@mask2);

-- Determine the number of slices we have to loop through
SET @slices = CEILING(@max_length/CAST(@slice_bytes AS FLOAT))

-- Loop through slices of our bitmasks
DECLARE @i INT;
SET @i = 0;
WHILE @i < @slices
BEGIN

-- Set @bin1 to the current INT sized slice of @mask1
IF(DATALENGTH(@mask1) > @i*@slice_bytes)
SET @bin1 = CAST(SUBSTRING(@mask1,DATALENGTH(@mask1)+1-(@i+1)*@slice_bytes,@slice_bytes) AS INT);
ELSE
SET @bin1 = 0;

-- Set @bin2 to the current INT sized slice of @mask2
IF(DATALENGTH(@mask2) > @i*@slice_bytes)
SET @bin2 = CAST(SUBSTRING(@mask2,DATALENGTH(@mask2)+1-(@i+1)*@slice_bytes,@slice_bytes) AS INT);
ELSE
SET @bin2 = 0;

-- Do bitwise operation and save in @result
IF(@operator = '&')
SET @result = @bin1 & @bin2;
ELSE
IF(@operator = '|')
SET @result = @bin1 | @bin2
ELSE
IF(@operator = '^')
SET @result = @bin1 ^ @bin2;
ELSE
RETURN 0x0;

-- Cast @result back to VARBINARY and append to @resultmask
IF(@resultmask IS NULL)
SET @resultmask = CAST(@result AS VARBINARY);
ELSE
SET @resultmask = CAST(@result AS VARBINARY) + @resultmask;

-- Increment counter
SET @i = @i + 1;
END

RETURN SUBSTRING(@resultmask,DATALENGTH(@resultmask)-@max_length+1,@max_length);
END;
GO
