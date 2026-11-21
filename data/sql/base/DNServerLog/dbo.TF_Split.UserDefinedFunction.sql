USE [dnserverlog]
GO
/****** Object:  UserDefinedFunction [dbo].[TF_Split]    Script Date: 11/14/2026 8:57:58 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-07-10
description : ???? ?? ? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[TF_Split] (
	@vchString varchar(max),
	@vchDelimiter varchar(10)
)
RETURNS @tblResult table (string varchar(8000) NOT NULL)
AS
BEGIN
DECLARE @xmlString xml;

SET @xmlString = CAST(N'<X>' + REPLACE(@vchString, @vchDelimiter, '</X><X>') + '</X>' AS xml);

INSERT @tblResult (string)
SELECT N.value('.', 'varchar(8000)') FROM @xmlString.nodes('X') AS T(N)

RETURN;
END
GO
