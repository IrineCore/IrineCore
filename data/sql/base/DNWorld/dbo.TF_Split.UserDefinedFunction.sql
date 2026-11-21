USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[TF_Split]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
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

	SET @xmlString = CAST('<X n="' + REPLACE(@vchString, @vchDelimiter, '"/><X n="') + '"/>' AS xml);

	INSERT @tblResult (string)
	SELECT T.col.value('@n', 'varchar(8000)') AS n FROM @xmlString.nodes('/X') T(col);

	RETURN;
END
GO
