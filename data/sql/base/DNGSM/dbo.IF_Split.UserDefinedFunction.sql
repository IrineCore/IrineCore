USE [dngsm]
GO
/****** Object:  UserDefinedFunction [dbo].[IF_Split]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-04-02
description : ???? ?? ? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[IF_Split] (
	@vchString nvarchar(max),
	@chrDelimiter nchar(1)
)
RETURNS TABLE
AS
RETURN (
	SELECT SUBSTRING(@chrDelimiter + @vchString + @chrDelimiter, n + 1, CHARINDEX(@chrDelimiter, @chrDelimiter + @vchString + @chrDelimiter, n + 1) - n - 1) AS string
	FROM dbo.Tally WITH (NOLOCK)
	WHERE n < DATALENGTH(@chrDelimiter + @vchString + @chrDelimiter) AND SUBSTRING(@chrDelimiter + @vchString + @chrDelimiter, n, 1) = @chrDelimiter
)
GO
