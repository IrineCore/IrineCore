USE [dnworld]
GO
/****** Object:  Table [dbo].[TotalPvPRooms]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TotalPvPRooms](
	[TotalPvPRoomID] [int] IDENTITY(1,1) NOT NULL,
	[WorldID] [int] NOT NULL,
	[ServerID] [int] NOT NULL,
	[RoomID] [bigint] NOT NULL,
	[PvPModeCode] [tinyint] NOT NULL,
	[TotalPvPRoomName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[MaxPlayerCount] [tinyint] NOT NULL,
	[MapID] [int] NOT NULL,
	[BreakIntoFlag] [bit] NOT NULL,
	[ItemPossibleFlag] [bit] NOT NULL,
	[ReviseFlag] [bit] NOT NULL,
	[ObserverFlag] [bit] NOT NULL,
	[RandomFlag] [bit] NOT NULL,
	[MinLevel] [tinyint] NOT NULL,
	[MaxLevel] [tinyint] NOT NULL,
	[PassWordValue] [int] NULL,
	[ChannelTypeCode] [tinyint] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[WinConditionCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_TotalPvPRooms] PRIMARY KEY CLUSTERED 
(
	[TotalPvPRoomID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
