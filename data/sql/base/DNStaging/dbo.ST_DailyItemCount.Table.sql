USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyItemCount]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyItemCount](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NULL,
	[ItemID] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemUseCode] [tinyint] NULL,
	[ItemOptionGroupCode] [tinyint] NOT NULL,
	[ActiveUserFlag] [bit] NOT NULL,
	[ItemLocationCode] [tinyint] NULL,
	[ItemCount] [int] NULL,
	[ItemCategoryID] [smallint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyItemCount_BasisDate_WorldID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyItemCount_BasisDate_WorldID] ON [dbo].[ST_DailyItemCount]
(
	[BasisDate] ASC,
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
GO
