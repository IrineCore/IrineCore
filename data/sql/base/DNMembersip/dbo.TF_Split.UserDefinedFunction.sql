USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[TF_Split]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TF_Split]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'/**
version : 5
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-04-02
description : ???? ?? ? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[TF_Split] (
	@vchString varchar(max),
	@vchDelimiter varchar(10)
)
RETURNS @tblResult table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, string varchar(8000) NOT NULL)
AS
BEGIN
	DECLARE @xmlString xml;

	SET @xmlString = CAST(''<X n="'' + REPLACE(@vchString, @vchDelimiter, ''"/><X n="'') + ''"/>'' AS xml);

	INSERT @tblResult (string)
	SELECT T.col.value(''@n'', ''varchar(8000)'') AS n FROM @xmlString.nodes(''/X'') T(col);

	RETURN;
END
' 
END
GO
