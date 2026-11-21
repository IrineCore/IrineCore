USE [dngsm]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_DynamicWhereClause2]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-01-23
description : Makes the [where clause] that is included in dynamic query.
**/

CREATE FUNCTION [dbo].[FN_DynamicWhereClause2] (
	@nvcColumnName nvarchar(128),
	@nvcVariableName nvarchar(128),
	@inbNullBitmap bigint,
	@intColumnSequence int,
	@intColumnCount int,
	@vchComparisonType varchar(10)
)
RETURNS nvarchar(max)
AS
BEGIN
	DECLARE @nvcWhereClause nvarchar(max)

	SET @nvcWhereClause =
		CASE
			WHEN @intColumnSequence = 1 AND @inbNullBitmap > 0 THEN ' WHERE '
			ELSE N''
		END +
		CASE
			WHEN @inbNullBitmap & POWER(CAST(2 AS bigint), @intColumnSequence - 1) = 0 THEN N''
			ELSE @nvcColumnName +
				CASE @vchComparisonType
					WHEN '=' THEN N' = ' + @nvcVariableName
					WHEN 'A%' THEN N' LIKE (' + @nvcVariableName + N' + ''%'')'
					WHEN '%A%' THEN N' LIKE (''%'' + ' + @nvcVariableName + N' + ''%'')'
					WHEN '>' THEN N' > ' + @nvcVariableName
					WHEN '>=' THEN N' >= ' + @nvcVariableName
					WHEN '<' THEN N' < ' + @nvcVariableName
					WHEN '<=' THEN N' <= ' + @nvcVariableName
					WHEN '<>' THEN N' <> ' + @nvcVariableName
					WHEN '&' THEN N' & '  + @nvcVariableName + N' > 0'
					ELSE N''
				END +
				CASE
					WHEN @inbNullBitmap & ((POWER(CAST(2 AS bigint), @intColumnCount) - 1) - (POWER(CAST(2 AS bigint), @intColumnSequence) - 1)) > 0 THEN N' AND '
					ELSE N''
				END
		END

	RETURN @nvcWhereClause
END
GO
