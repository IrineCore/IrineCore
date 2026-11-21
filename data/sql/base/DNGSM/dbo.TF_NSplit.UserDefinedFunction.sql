USE [dngsm]
GO
/****** Object:  UserDefinedFunction [dbo].[TF_NSplit]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2008-07-10
description : ???? ?? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[TF_NSplit] (
	@nvcString nvarchar(max),
	@nvcDelimiter nvarchar(10)
)
RETURNS @tblResult table (string nvarchar(4000) NOT NULL)
AS
BEGIN
DECLARE @xmlString xml;

SET @xmlString = CAST(N'<X>'+REPLACE(@nvcString, @nvcDelimiter, N'</X><X>')+N'</X>' AS xml);

INSERT @tblResult (string)
SELECT N.value(N'.', 'nvarchar(4000)') FROM @xmlString.nodes(N'X') AS T(N)

RETURN;
END
GO
