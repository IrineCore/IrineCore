USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_DynamicWhereClause]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2008-02-19
description : Makes the [where clause] that is included in dynamic query.
**/

CREATE FUNCTION [dbo].[FN_DynamicWhereClause] (
	@nvcColumnName nvarchar(128),
	@nvcVariableName nvarchar(128),
	@inbNullBitmap bigint,
	@intColumnSequence int,
	@intColumnCount int,
	@inyComparisonType tinyint
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
				CASE @inyComparisonType
					WHEN 1 THEN N' = ' + @nvcVariableName
					WHEN 2 THEN N' LIKE (' + @nvcVariableName + N' + ''%'')'
					WHEN 3 THEN N' LIKE (''%'' + ' + @nvcVariableName + N' + ''%'')'
					WHEN 4 THEN N' > ' + @nvcVariableName
					WHEN 5 THEN N' >= ' + @nvcVariableName
					WHEN 6 THEN N' < ' + @nvcVariableName
					WHEN 7 THEN N' <= ' + @nvcVariableName
					WHEN 8 THEN N' <> ' + @nvcVariableName
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
