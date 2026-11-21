USE [dnmembership]
GO
/****** Object:  Table [dbo].[DNEventSchedules]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DNEventSchedules]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DNEventSchedules](
	[EventEndDate] [smalldatetime] NOT NULL,
	[EventStartDate] [smalldatetime] NOT NULL,
	[DNEventID] [int] NOT NULL,
 CONSTRAINT [PK_DNEventSchedules] PRIMARY KEY CLUSTERED 
(
	[EventEndDate] ASC,
	[EventStartDate] ASC,
	[DNEventID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_DNEvents_DNEventSchedules_DNEventID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNEventSchedules]'))
ALTER TABLE [dbo].[DNEventSchedules]  WITH NOCHECK ADD  CONSTRAINT [FK_DNEvents_DNEventSchedules_DNEventID] FOREIGN KEY([DNEventID])
REFERENCES [dbo].[DNEvents] ([DNEventID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_DNEvents_DNEventSchedules_DNEventID]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNEventSchedules]'))
ALTER TABLE [dbo].[DNEventSchedules] CHECK CONSTRAINT [FK_DNEvents_DNEventSchedules_DNEventID]
GO
