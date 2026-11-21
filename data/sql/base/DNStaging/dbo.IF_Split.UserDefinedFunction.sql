USE [dnstaging]
GO
/****** Object:  UserDefinedFunction [dbo].[IF_Split]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-04-02
description : ???? ?? ? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[IF_Split] (
	@vchString varchar(max),
	@chrDelimiter char(1)
)
RETURNS TABLE
AS
RETURN (
	SELECT ROW_NUMBER() OVER(ORDER BY n) AS seq, SUBSTRING(@chrDelimiter + @vchString + @chrDelimiter, n + 1, CHARINDEX(@chrDelimiter, @chrDelimiter + @vchString + @chrDelimiter, n + 1) - n - 1) AS string
	FROM dbo.Tally WITH (NOLOCK)
	WHERE n < DATALENGTH(@chrDelimiter + @vchString + @chrDelimiter) AND SUBSTRING(@chrDelimiter + @vchString + @chrDelimiter, n, 1) = @chrDelimiter
)
GO
