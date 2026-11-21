USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_MonthlyUV]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_MonthlyUV](
	[BasisDate] [date] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[UVCount] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_MonthlyUV_BasisDate]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NC_ST_MonthlyUV_BasisDate] ON [dbo].[ST_MonthlyUV]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
