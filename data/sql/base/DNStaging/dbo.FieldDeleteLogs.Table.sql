USE [dnstaging]
GO
/****** Object:  Table [dbo].[FieldDeleteLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldDeleteLogs](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[PlantDate] [datetime2](0) NOT NULL,
	[ElapsedTime] [int] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
