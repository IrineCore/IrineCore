USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[TF_NSplit]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-04-02
description : ???? ?? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[TF_NSplit] (
	@nvcString nvarchar(max),
	@nvcDelimiter nvarchar(10)
)
RETURNS @tblResult table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, string nvarchar(4000) NOT NULL)
AS
BEGIN
	DECLARE @xmlString xml;

	SET @xmlString = CAST(N'<X n="' + REPLACE(@nvcString, @nvcDelimiter, N'"/><X n="') + N'"/>' AS xml);

	INSERT @tblResult (string)
	SELECT T.col.value('@n', 'nvarchar(4000)') AS n FROM @xmlString.nodes('/X') T(col);

	RETURN;
END
GO
