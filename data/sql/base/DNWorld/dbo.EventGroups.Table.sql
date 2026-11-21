USE [dnworld]
GO
/****** Object:  Table [dbo].[EventGroups]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EventGroups](
	[EventGroupID] [int] IDENTITY(1,1) NOT NULL,
	[EventName] [nvarchar](200) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[EventStartDate] [datetime] NOT NULL,
	[EventEndDate] [datetime] NOT NULL,
	[EventDescription] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NULL,
	[RegisterID] [int] NOT NULL,
	[RegisteDate] [datetime] NOT NULL,
 CONSTRAINT [PK_EventGroups] PRIMARY KEY CLUSTERED 
(
	[EventGroupID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
