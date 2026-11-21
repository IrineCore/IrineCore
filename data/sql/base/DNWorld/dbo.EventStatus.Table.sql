USE [dnworld]
GO
/****** Object:  Table [dbo].[EventStatus]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EventStatus](
	[EventID] [int] NULL,
	[WorldID] [int] NOT NULL,
	[MapIDs] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[EventStartDate] [datetime] NOT NULL,
	[EventEndDate] [datetime] NOT NULL,
	[EventType1] [int] NOT NULL,
	[EventType2] [int] NOT NULL,
	[EventType3] [int] NOT NULL,
	[EventAtt1] [int] NOT NULL,
	[EventAtt2] [int] NOT NULL,
	[EventAtt3] [int] NOT NULL,
	[EventAtt4] [int] NOT NULL,
	[EventAtt5] [int] NOT NULL,
	[EventGroupID] [int] NULL,
	[EventIndex] [int] IDENTITY(1,1) NOT NULL,
 CONSTRAINT [PK_EventStatus] PRIMARY KEY CLUSTERED 
(
	[EventIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
